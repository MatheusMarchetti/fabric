#include "core/application.hpp"
#include "core/clock.hpp"
#include "core/engine.hpp"
#include "core/event.hpp"
#include "core/input.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"
#include "platform/platform.hpp"
#include "renderer/renderer.hpp"

using namespace fabric;

namespace {
    enum application_state : u8 {
        terminating = 0,
        running = (1u << 0),
        suspended = (1u << 1),
    };

    static b8 is_initialized = false;
    application_state current_state;
    platform::window platform_state;

    f64 last_time;
}  // namespace

b8 on_quit(u16 code, void* sender, void* listener_inst, event::context context);
b8 on_key(u16 code, void* sender, void* listener_inst, event::context context);
b8 on_resize(u16 code, void* sender, void* listener_inst, event::context context);

b8 fabric::initialize(application& app) {
    if (is_initialized) {
        FBWARN("fabric::initialize cannot be called more than once.");

        return false;
    }

    platform_state.width = app.config.client_width;
    platform_state.height = app.config.client_height;
    platform_state.x = app.config.posX;
    platform_state.y = app.config.posY;
    platform_state.name = app.config.name;
    current_state = application_state::running;

    logger::initialize();

    if (!memory::initialize()) {
        FBERROR("An error ocurred during memory system initialization.");
        current_state = application_state::terminating;
        return false;
    }

    if (!event::initialize()) {
        FBERROR("An error ocurred during event system initialization.");
        current_state = application_state::terminating;
        return false;
    }

    if (!platform::initialize(platform_state)) {
        FBERROR("An error ocurred during platform system initialization.");
        current_state = application_state::terminating;
        return false;
    }

    if (!renderer::initialize(platform_state)) {
        FBERROR("An error ocurred during renderer system initialization.");
        current_state = application_state::terminating;
        return false;
    }

    event::checkin(event::APPLICATION_QUIT, 0, on_quit);
    event::checkin(event::KEY_PRESSED, 0, on_key);
    event::checkin(event::RESIZED, 0, on_resize);

    if (!input::initialize()) {
        FBERROR("An error ocurred during input system initialization.");
        current_state = application_state::terminating;
        return false;
    }

    if (app.initialize) {
        if (!app.initialize()) {
            FBERROR("An error ocurred during application initialization.");
            current_state = application_state::terminating;
            return false;
        }
    }

    FBINFO("Fabric engine initialization successful.");
    return is_initialized = true;
}

b8 fabric::update(application& app) {
    ftl::stopwatch clock;
    last_time = clock.mark();
    // f64 running_time = 0.0;
    f64 target_frame_seconds = 1.0 / 60.0;

    while (current_state != application_state::terminating) {
        if (!platform::update(platform_state)) {
            FBERROR("An error ocurred during platform update.");
            current_state = application_state::terminating;
            return false;
        }

        if (current_state == application_state::running) {
            f64 current_time = clock.mark();
            f64 delta = (current_time - last_time);
            f64 frame_start_time = platform::get_absolute_time();

            if (app.begin_frame) {
                if (!app.begin_frame(delta)) {
                    FBERROR("An error ocurred during application frame start.");
                    current_state = application_state::terminating;
                    return false;
                }
            }

            // TODO: Process internal frame logic

            if (app.end_frame) {
                if (!app.end_frame(delta)) {
                    FBERROR("An error ocurred during application frame end.");
                    current_state = application_state::terminating;
                    return false;
                }
            }

            if (!renderer::draw_frame(delta)) {
                FBERROR("An error ocurred during renderer::draw_frame.");
                current_state = application_state::terminating;
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

            last_time = current_time;
        }
    }

    return true;
}

void fabric::terminate(application& app) {
    if (current_state == application_state::terminating) {
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
        platform::terminate(platform_state);
    } else {
        FBFATAL("How the hell we wind up like this?!");
    }
}

b8 on_quit(u16 code, void* sender, void* listener_inst, event::context context) {
    if (code == event::APPLICATION_QUIT) {
        FBINFO("Received APPLICATION_QUIT event. Terminating.");
        current_state = application_state::terminating;
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

        current_state = application_state::running;

        if (width == 0 || height == 0) {
            current_state = application_state::suspended;
            return true;
        }

        renderer::resize(width, height);

        return true;
    }

    return false;
}