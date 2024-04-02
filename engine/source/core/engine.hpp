#pragma once

#include "defines.hpp"

struct application;

namespace fabric {
    extern "C" {
        FB_API b8 initialize(application& app);
        FB_API b8 update(application& app);
        FB_API void terminate(application& app);
    }
}