#include <core/logger.hpp>
#include <core/asserts.hpp>

int main() {
    FBFATAL("A test message: %f", 3.14f);
    FBERROR("A test message: %f", 3.14f);
    FBWARN("A test message: %f", 3.14f);
    FBINFO("A test message: %f", 3.14f);
    FBDEBUG("A test message: %f", 3.14f);

    FBASSERT(1 == 0);

    return 0;
}