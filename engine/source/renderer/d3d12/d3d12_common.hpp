#pragma once

#include "renderer/d3d12/d3d12_defines.hpp"

#define NOMINMAX
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

static constexpr u8 frames_in_flight = 2;
static constexpr u8 swapchain_image_count = 3;