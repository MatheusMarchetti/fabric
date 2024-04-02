#include "launcher.hpp"

bool initialize() {
    FBDEBUG("Initializing launcher application!");

    return true;
}

bool begin_frame(double timestep) {
    return true;
}

void terminate() {
    FBDEBUG("Terminating launcher application");
}