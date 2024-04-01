#include <core/logger.hpp>
#include <core/asserts.hpp>
#include <platform/platform.hpp>

using namespace fabric;

int main() {
    FBFATAL("A test message: %f", 3.14f);
    FBERROR("A test message: %f", 3.14f);
    FBWARN("A test message: %f", 3.14f);
    FBINFO("A test message: %f", 3.14f);
    FBDEBUG("A test message: %f", 3.14f);

    platform::state state;
    if (initialize(&state, "Fabric Engine Launcher", 100, 100, 1280, 720)) {
        while (true) {
            pump_messages(&state);
        }
    }
    shutdown(&state);

    return 0;
}