#include "launcher.hpp"

using namespace fabric;

bool launcher_initialize() {
    FBDEBUG("Initializing launcher application");
    return true;
}

bool begin_frame(double timestep) {
    return true;
}

void launcher_terminate() {
    FBDEBUG("Terminating launcher application");
}