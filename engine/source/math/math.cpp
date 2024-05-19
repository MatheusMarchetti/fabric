#include "math/math.hpp"
#include "platform/platform.hpp"

using namespace fabric;

#include <math.h>
#include <stdlib.h>

f32 ftl::fbsin(f32 x) {
    return sinf(x);
}

f32 ftl::fbcos(f32 x) {
    return cosf(x);
}

f32 ftl::fbtan(f32 x) {
    return tanf(x);
}

f32 ftl::fbacos(f32 x) {
    return acosf(x);
}

f32 ftl::fbsqrt(f32 x) {
    return sqrtf(x);
}

f32 ftl::fbabs(f32 x) {
    return fabsf(x);
}

i32 ftl::fbrandom(u32 seed, i32 min, i32 max) {
    u32 rng_seed = seed;
    if (seed == -1) {
        rng_seed = (u32)platform::get_absolute_time();
    }
    srand(rng_seed);

    if (max != min) {
        return (rand() % (max - min + 1)) + min;
    }

    return rand();
}

f32 ftl::fbrandom(u32 seed, f32 min, f32 max) {
    if ((max - min) > FB_FLOAT_EPSILON) {
        return min + ((f32)fbrandom(seed, (i32)min, (i32)max) / ((f32)RAND_MAX / (max - min)));
    }

    return (f32)fbrandom(seed, (i32)min, (i32)max) / (f32)RAND_MAX;
}

u64 ftl::max(u64 val1, u64 val2) {
    return (val1 > val2) ? val1 : val2;
}

u64 ftl::min(u64 val1, u64 val2) {
    return (val1 < val2) ? val1 : val2; 
}