#include "core/application.hpp"
#include "core/engine.hpp"
#include "core/event.hpp"
#include "core/input.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"
#include "platform/platform.hpp"

using namespace fabric;

namespace {
    enum application_state : u8 {
        terminating = 0,
        running = (1u << 0),
        suspended = (1u << 1),
    };

    static b8 is_initialized = false;
    application_state current_state;
    platform::state platform_state;
    i16 width;
    i16 height;
    f64 last_time;
}  // namespace

b8 on_event(u16 code, void* sender, void* listener_inst, event::context context);
b8 on_key(u16 code, void* sender, void* listener_inst, event::context context);

namespace fabric {
    b8 initialize(application& app) {
        if (is_initialized) {
            FBWARN("fabric::initialize cannot be called more than once.");

            return false;
        }
        width = app.config.client_width;
        height = app.config.client_height;
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

        event::checkin(event::APPLICATION_QUIT, 0, on_event);
        event::checkin(event::KEY_PRESSED, 0, on_key);
        event::checkin(event::KEY_RELEASED, 0, on_key);

        if (!input::initialize()) {
            FBERROR("An error ocurred during input system initialization.");
            current_state = application_state::terminating;
            return false;
        }

        if (!platform::initialize(&platform_state, app.config.name, app.config.posX, app.config.posY, width, height)) {
            FBERROR("An error ocurred during platform system initialization.");
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

        last_time = platform::get_absolute_time();

        FBINFO("Fabric engine initialization successful.");
        return is_initialized = true;
    }

    b8 update(application& app) {
        while (current_state != application_state::terminating) {
            if (!platform::update(&platform_state)) {
                FBERROR("An error ocurred during platform update.");
                current_state = application_state::terminating;
                return false;
            }

            f64 time_now = platform::get_absolute_time();
            f64 timestep = time_now - last_time;
            last_time = time_now;

            if (current_state == application_state::running) {
                if (app.begin_frame) {
                    if (!app.begin_frame(timestep)) {
                        FBERROR("An error ocurred during application frame start.");
                        current_state = application_state::terminating;
                        return false;
                    }
                }

                // TODO: Process internal frame logic

                if (app.end_frame) {
                    if (!app.end_frame(timestep)) {
                        FBERROR("An error ocurred during application frame end.");
                        current_state = application_state::terminating;
                        return false;
                    }
                }

                // NOTE: As far as the application is concerned, a "frame" is the logic update phase. A frame for the renderer is not related to this.
                // TODO: Start rendering

                input::update(timestep);
            }
        }

        return true;
    }

    void terminate(application& app) {
        if (current_state == application_state::terminating) {
            if (app.terminate) {
                app.terminate();
            }

            event::checkout(event::APPLICATION_QUIT, 0, on_event);
            event::checkout(event::KEY_PRESSED, 0, on_key);
            event::checkout(event::KEY_RELEASED, 0, on_key);

            input::terminate();
            event::terminate();
            memory::terminate();
            logger::terminate();
            platform::terminate(&platform_state);
        } else {
            FBFATAL("How the hell we wind up like this?!");
        }
    }
}  // namespace fabric

b8 on_event(u16 code, void* sender, void* listener_inst, event::context context) {
    switch (code) {
        case event::APPLICATION_QUIT:
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
        } else if (key == input::KEY_A) {
            FBDEBUG("Explicit - A key pressed");
            return true;
        } else {
            FBDEBUG("'%c' key pressed in window.", key);
            return true;
        }
    }

    return false;
}