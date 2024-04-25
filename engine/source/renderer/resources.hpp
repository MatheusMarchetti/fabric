#pragma once

#include "defines.hpp"
#include "core/handle.hpp"

namespace fabric {
    struct texture {
        enum flags {
            readonly = 0x01,
            writable = 0x02,
            render_target = 0x04,
            depth_stencil = 0x08
        };

    };

    template<>
    struct handle<texture> {
        u32 index : 20;
        u32 shader_resource : 20;
        u32 render_target : 10;
        u32 depth_stencil : 10;
        u32 flags : 4;
    };

    struct buffer {};
}  // namespace fabric