#include <fabric.hpp>
#include <entry_point.hpp>

#include "launcher.hpp"

void create_application(application& app) {
    app.config = {
        .name = "Fabric Launcher",
        .posX = 100,
        .posY = 100,
        .client_width = 1280,
        .client_height = 720};
    
    app.initialize = initialize;
    app.begin_frame = begin_frame;
    app.terminate = terminate;
}