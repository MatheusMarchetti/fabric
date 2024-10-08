#pragma once

#include "defines.hpp"
#include "platform/platform.hpp"

namespace fabric::renderer {
    b8 initialize(u64& memory_requirement, void* memory, platform::window& window);
    void terminate();

    void resize(u16 width, u16 height);

    b8 draw_frame(f64 timestep);
}