#include "renderer/renderer.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"

// TODO: Move to separate library and load dynamically
#include "renderer/d3d12/d3d12_interface.hpp"

using namespace fabric;

namespace {
    typedef b8 (*initialize_pfn)(void*);
    typedef void (*terminate_pfn)();
    typedef void (*begin_frame_pfn)();
    typedef void (*end_frame_pfn)();
    typedef void(*resize_pfn)(u16, u16);
    typedef void(*update_pfn)(f64);
    typedef b8(*present_pfn)();

    struct system_state {
        initialize_pfn initialize;
        terminate_pfn terminate;
        begin_frame_pfn begin_frame;
        end_frame_pfn end_frame;
        resize_pfn resize;
        update_pfn update;
        present_pfn present;
    };

    static system_state* state;
}  // namespace

b8 renderer::initialize(u64& memory_requirement, void* memory, platform::window& window) {
    memory_requirement = sizeof(system_state);
    if(!memory) {
        return true;
    }

    if (state) {
        FBERROR("Renderer system was already initialized!");
        return false;
    }
    state = (system_state*)memory;
    memory::fbzero(state, sizeof(system_state));

    // TODO: API backends should be separated in multiple dynamic libraries. This is where they would be loaded and their methods assigned
    //       to the common interface declared in renderer_backend

    state->initialize = d3d12_initialize;
    state->terminate = d3d12_terminate;
    state->begin_frame = d3d12_begin_frame;
    state->end_frame = d3d12_end_frame;
    state->resize = d3d12_resize;
    state->update = d3d12_update;
    state->present = d3d12_present;

    if(!state->initialize(&window)) {
        return false;
    }

    FBINFO("Renderer system initialized.");

    return true;
}

void renderer::terminate() {
    state->terminate();

    state = nullptr;
}

void renderer::resize(u16 width, u16 height) {
    state->resize(width, height);
}

b8 renderer::draw_frame(f64 timestep) {
    state->begin_frame();

    state->update(timestep);

    state->end_frame();

    return state->present();
}