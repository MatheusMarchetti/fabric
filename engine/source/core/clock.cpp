#include "core/clock.hpp"
#include "platform/platform.hpp"

using namespace fabric;

ftl::stopwatch::stopwatch() {
    start_time = platform::get_absolute_time();
    elapsed = 0.0;
}

f64 ftl::stopwatch::mark() {
    elapsed = platform::get_absolute_time() - start_time;

    return elapsed;
}