#pragma once

#include "defines.hpp"
#include "core/asserts.hpp"
#include "comdef.h"

#define HRCheck(expr)                                                         \
    {                                                                         \
        HRESULT hresult = expr;                                               \
        if (hresult != S_OK) {                                                \
            _com_error error(hresult);                                        \
            FBASSERT_MSG(hresult == S_OK, (const char*)error.ErrorMessage()); \
        }                                                                     \
    }

#define NAME_OBJECTS(obj)    \
    {                        \
        obj->SetName(L#obj); \
    }

#define NAME_OBJECTS_TYPE(obj, type) \
    {                                \
        obj->SetName(L#type);        \
    }

#define NOMINMAX
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

static constexpr u8 frames_in_flight = 2;
static constexpr u8 swapchain_image_count = 3;

namespace fabric {
    struct descriptor_allocation {
        u32 offset;
        D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    };
}  // namespace fabric