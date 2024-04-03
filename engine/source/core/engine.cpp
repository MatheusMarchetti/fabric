#include "core/application.hpp"
#include "core/engine.hpp"
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

    static b8 initialized = false;
    application_state current_state;
    platform::state platform_state;
    i16 width;
    i16 height;
    f64 last_time;
}  // namespace

namespace fabric {
    b8 initialize(application& app) {
        if (initialized) {
            FBWARN("fabric::initialize cannot be called more than once.");

            return false;
        }
        width = app.config.client_width;
        height = app.config.client_height;
        current_state = application_state::running;

        memory::initialize();
        logger::initialize();

        if (!platform::initialize(&platform_state, app.config.name, app.config.posX, app.config.posY, width, height)) {
            FBERROR("An error ocurred during platform initialization.");
            current_state = application_state::terminating;
            return false;
        }

        if(app.initialize) {
            if(!app.initialize()) {
                FBERROR("An error ocurred during application initialization.");
                current_state = application_state::terminating;
                return false;
            }
        }

        last_time = platform::get_absolute_time();

        initialized = true;
        return true;
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
                if(app.begin_frame) {
                    if(!app.begin_frame(timestep)) {
                        FBERROR("An error ocurred during application frame start.");
                        current_state = application_state::terminating;
                        return false;
                    }
                }

                // TODO: Process internal frame logic

                 if(app.end_frame) {
                    if(!app.end_frame(timestep)) {
                        FBERROR("An error ocurred during application frame end.");
                        current_state = application_state::terminating;
                        return false;
                    }
                }

                // NOTE: As far as the application is concerned, a "frame" is the logic update phase. A frame for the renderer is not related to this.
                // TODO: Start rendering
            }
        }

        return true;
    }

    void terminate(application& app) {
        if (current_state == application_state::terminating) {
            if(app.terminate) {
                app.terminate();
            }

            // TODO: terminate all subsystems before logger, except platform

            logger::terminate();
            memory::terminate();

            platform::terminate(&platform_state);
        } else {
            FBFATAL("How the hell did we end up here?!");
        }
    }
}  // namespace fabric