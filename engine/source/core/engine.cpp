#include "core/application.hpp"
#include "core/engine.hpp"
#include "core/event.hpp"
#include "core/input.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"
#include "ftl/clock.hpp"
#include "memory/linear_allocator.hpp"
#include "platform/platform.hpp"
#include "renderer/renderer.hpp"

using namespace fabric;

namespace {
    enum application_status : u8 {
        terminating = 0,
        running = (1u << 0),
        suspended = (1u << 1),
    };

    struct application_state {
        application_status current_status;
        platform::window window;
        memory::linear_allocator internal_systems;
        f64 last_time;
    };

    static application_state* state;

}  // namespace

b8 on_quit(u16 code, void* sender, void* listener_inst, event::context context);
b8 on_key(u16 code, void* sender, void* listener_inst, event::context context);
b8 on_resize(u16 code, void* sender, void* listener_inst, event::context context);

b8 fabric::initialize(application& app) {
    if (state) {
        FBWARN("fabric::initialize cannot be called more than once.");

        return false;
    }

    app.internal_state = memory::fballocate(sizeof(application_state), memory::MEMORY_TAG_APPLICATION);
    state = (application_state*)app.internal_state;

    state->window.width = app.config.client_width;
    state->window.height = app.config.client_height;
    state->window.x = app.config.posX;
    state->window.y = app.config.posY;
    state->window.name = app.config.name;

    // Query for internal systems size

    u64 internal_systems_memory_requirement = 0;
    u64 memory_system_memory_requirement;
    u64 logger_system_memory_requirement;
    u64 event_system_memory_requirement;
    u64 input_system_memory_requirement;
    u64 platform_system_memory_requirement;
    u64 renderer_system_memory_requirement;

    {
        memory::initialize(memory_system_memory_requirement, nullptr);
        internal_systems_memory_requirement += memory_system_memory_requirement;

        logger::initialize(logger_system_memory_requirement, nullptr);
        internal_systems_memory_requirement += logger_system_memory_requirement;

        event::initialize(event_system_memory_requirement, nullptr);
        internal_systems_memory_requirement += event_system_memory_requirement;

        input::initialize(input_system_memory_requirement, nullptr);
        internal_systems_memory_requirement += input_system_memory_requirement;

        platform::initialize(platform_system_memory_requirement, nullptr, state->window);
        internal_systems_memory_requirement += platform_system_memory_requirement;

        renderer::initialize(renderer_system_memory_requirement, nullptr, state->window);
        internal_systems_memory_requirement += renderer_system_memory_requirement;
    }

    state->internal_systems.create(internal_systems_memory_requirement);

    // Initialize internal systems
    {
        if (!memory::initialize(memory_system_memory_requirement, state->internal_systems.allocate(memory_system_memory_requirement))) {
            FBERROR("An error ocurred during memory system initialization.");
            state->current_status = application_status::terminating;
            return false;
        }

        if (!logger::initialize(logger_system_memory_requirement, state->internal_systems.allocate(logger_system_memory_requirement))) {
            FBERROR("An error ocurred during logger system initialization.");
            state->current_status = application_status::terminating;
            return false;
        }

        if (!event::initialize(event_system_memory_requirement, state->internal_systems.allocate(event_system_memory_requirement))) {
            FBERROR("An error ocurred during event system initialization.");
            state->current_status = application_status::terminating;
            return false;
        }

        if (!input::initialize(input_system_memory_requirement, state->internal_systems.allocate(input_system_memory_requirement))) {
            FBERROR("An error ocurred during input system initialization.");
            state->current_status = application_status::terminating;
            return false;
        }

        if (!platform::initialize(platform_system_memory_requirement, state->internal_systems.allocate(platform_system_memory_requirement), state->window)) {
            FBERROR("An error ocurred during platform system initialization.");
            state->current_status = application_status::terminating;
            return false;
        }

        if (!renderer::initialize(renderer_system_memory_requirement, state->internal_systems.allocate(renderer_system_memory_requirement), state->window)) {
            FBERROR("An error ocurred during renderer system initialization.");
            state->current_status = application_status::terminating;
            return false;
        }
    }

    event::checkin(event::APPLICATION_QUIT, 0, on_quit);
    event::checkin(event::KEY_PRESSED, 0, on_key);
    event::checkin(event::RESIZED, 0, on_resize);

    if (app.initialize) {
        if (!app.initialize()) {
            FBERROR("An error ocurred during application initialization.");
            state->current_status = application_status::terminating;
            return false;
        }
    }

    FBINFO("Fabric engine initialization successful.");

    state->current_status = application_status::running;
    return true;
}

b8 fabric::update(application& app) {
    ftl::stopwatch clock;
    state->last_time = clock.mark();
    // f64 running_time = 0.0;
    f64 target_frame_seconds = 1.0 / 60.0;

    while (state->current_status != application_status::terminating) {
        if (!platform::update(state->window)) {
            FBERROR("An error ocurred during platform update.");
            state->current_status = application_status::terminating;
            return false;
        }

        if (state->current_status == application_status::running) {
            f64 current_time = clock.mark();
            f64 delta = (current_time - state->last_time);
            f64 frame_start_time = platform::get_absolute_time();

            if (app.begin_frame) {
                if (!app.begin_frame(delta)) {
                    FBERROR("An error ocurred during application frame start.");
                    state->current_status = application_status::terminating;
                    return false;
                }
            }

            // TODO: Process internal frame logic

            if (app.end_frame) {
                if (!app.end_frame(delta)) {
                    FBERROR("An error ocurred during application frame end.");
                    state->current_status = application_status::terminating;
                    return false;
                }
            }

            if (!renderer::draw_frame(delta)) {
                FBERROR("An error ocurred during renderer::draw_frame.");
                state->current_status = application_status::terminating;
                return false;
            }

            f64 frame_end_time = platform::get_absolute_time();
            f64 frame_elapsed_time = frame_end_time - frame_start_time;
            // running_time += frame_elapsed_time;
            f64 remaining_seconds = target_frame_seconds - frame_elapsed_time;

            if (remaining_seconds > 0.0) {
                u64 remaining_millis = (remaining_seconds * 1000);

                if (remaining_millis > 0) {
                    platform::sleep(remaining_millis - 1);
                }
            }

            input::update(delta);

            state->last_time = current_time;
        }
    }

    return true;
}

void fabric::terminate(application& app) {
    if (state->current_status == application_status::terminating) {
        if (app.terminate) {
            app.terminate();
        }

        event::checkout(event::APPLICATION_QUIT, 0, on_quit);
        event::checkout(event::KEY_PRESSED, 0, on_key);
        event::checkout(event::RESIZED, 0, on_resize);

        input::terminate();
        renderer::terminate();
        event::terminate();
        memory::terminate();
        logger::terminate();
        platform::terminate(state->window);

        state->internal_systems.destroy();
    } else {
        FBFATAL("How the hell we wind up like this?!");
    }

    memory::fbfree(app.internal_state, sizeof(application_state), memory::MEMORY_TAG_APPLICATION);
}

b8 on_quit(u16 code, void* sender, void* listener_inst, event::context context) {
    if (code == event::APPLICATION_QUIT) {
        FBINFO("Received APPLICATION_QUIT event. Terminating.");
        state->current_status = application_status::terminating;
        return true;
    }

    return false;
}

b8 on_key(u16 code, void* sender, void* listener_inst, event::context context) {
    if (code == event::KEY_PRESSED) {
        u16 key = context.data.u16[0];
        if (key == input::keys::KEY_ESCAPE) {
            event::context data = {};
            event::send(event::APPLICATION_QUIT, nullptr, data);

            return true;
        }
    }

    return false;
}

b8 on_resize(u16 code, void* sender, void* listener_inst, event::context context) {
    if (code == event::RESIZED) {
        u16 width = context.data.u16[0];
        u16 height = context.data.u16[1];

        state->current_status = application_status::running;

        if (width == 0 || height == 0) {
            state->current_status = application_status::suspended;
            return true;
        }

        renderer::resize(width, height);

        return true;
    }

    return false;
}