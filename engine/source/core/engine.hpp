#pragma once

#include "defines.hpp"

struct application;

namespace fabric {
    FBAPI b8 initialize(application& app);
    FBAPI b8 update(application& app);
    FBAPI void terminate(application& app);
}  // namespace fabric