#include "launcher.hpp"

using namespace fabric;

bool launcher_initialize() {
    FBDEBUG("Initializing launcher application");

    memory::log_memory_usage();
    return true;
}

bool begin_frame(double timestep) {
    return true;
}

void launcher_terminate() {
    FBDEBUG("Terminating launcher application");
}