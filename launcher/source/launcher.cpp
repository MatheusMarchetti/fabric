#include "launcher.hpp"

using namespace fabric;

bool launcher_initialize() {
    FBDEBUG("Initializing launcher application!");

    void* block = memory::fballocate(35000, memory::MEMORY_TAG_APPLICATION);
    memory::log_memory_usage();

    memory::fbfree(block, 35000, memory::MEMORY_TAG_APPLICATION);
    memory::log_memory_usage();

    return true;
}

bool begin_frame(double timestep) {
    return true;
}

void launcher_terminate() {
    FBDEBUG("Terminating launcher application");
}