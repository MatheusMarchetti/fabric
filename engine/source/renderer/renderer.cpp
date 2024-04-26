#include "renderer/renderer.hpp"
#include "core/logger.hpp"

// TODO: Move to separate library and load dynamically
#include "renderer/d3d12/d3d12_backend.hpp"

using namespace fabric;

namespace {
    typedef b8 (*initialize_pfn)(void*);
    typedef void (*terminate_pfn)();
    typedef void (*begin_frame_pfn)();
    typedef void (*end_frame_pfn)();
    typedef void(*resize_pfn)(u16, u16);
    typedef void(*update_pfn)(f64);
    typedef b8(*present_pfn)();

    struct renderer_backend {
        initialize_pfn initialize;
        terminate_pfn terminate;
        begin_frame_pfn begin_frame;
        end_frame_pfn end_frame;
        resize_pfn resize;
        update_pfn update;
        present_pfn present;
    };

    static renderer_backend loaded_backend;
}  // namespace

b8 renderer::initialize(platform::window& window) {
    // TODO: API backends should be separated in multiple dynamic libraries. This is where they would be loaded and their methods assigned
    //       to the common interface declared in renderer_backend

    loaded_backend.initialize = d3d12_initialize;
    loaded_backend.terminate = d3d12_terminate;
    loaded_backend.begin_frame = d3d12_begin_frame;
    loaded_backend.end_frame = d3d12_end_frame;
    loaded_backend.resize = d3d12_resize;
    loaded_backend.update = d3d12_update;
    loaded_backend.present = d3d12_present;

    if(!loaded_backend.initialize(&window)) {
        return false;
    }

    FBINFO("Renderer system initialized.");

    return true;
}

void renderer::terminate() {
    loaded_backend.terminate();
}

void renderer::resize(u16 width, u16 height) {
    loaded_backend.resize(width, height);
}

b8 renderer::draw_frame(f64 timestep) {
    loaded_backend.begin_frame();

    loaded_backend.update(timestep);

    loaded_backend.end_frame();

    return loaded_backend.present();
}