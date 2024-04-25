#pragma once

#include "defines.hpp"
#include "core/handle.hpp"

namespace fabric {
    struct texture {};

    template<>
    struct handle<texture> {
        u32 index : 20;
        u32 shader : 20;
        u32 render_target : 10;
        u32 depth : 10;
        u32 flags : 4;
    };

    struct buffer {};
}  // namespace fabric