#include "renderer/renderer.hpp"
#include "core/logger.hpp"

// TODO: Move to separate library and load dynamically
#include "renderer/d3d12/backend_d3d12.hpp"

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

    static renderer_backend backend;
}  // namespace

b8 renderer::initialize(platform::window& window) {
    // TODO: API backends should be separated in multiple dynamic libraries. This is where they would be loaded and their methods assigned
    //       to the common interface declared in renderer_backend

    backend.initialize = d3d12_initialize;
    backend.terminate = d3d12_terminate;
    backend.begin_frame = d3d12_begin_frame;
    backend.end_frame = d3d12_end_frame;
    backend.resize = d3d12_resize;
    backend.update = d3d12_update;
    backend.present = d3d12_present;

    if(!backend.initialize(&window)) {
        return false;
    }

    FBINFO("Renderer system initialized.");

    return true;
}

void renderer::terminate() {
    backend.terminate();
}

void renderer::resize(u16 width, u16 height) {
    backend.resize(width, height);
}

b8 renderer::draw_frame(f64 timestep) {
    backend.begin_frame();

    backend.update(timestep);

    backend.end_frame();

    return backend.present();
}