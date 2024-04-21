#pragma once

#include "defines.hpp"
#include "renderer/d3d12/d3d12_common.hpp"

#include "renderer/resources.hpp"

namespace fabric::d3d12_device {
    b8 create(IDXGIFactory7* dxgiFactory);
    void destroy();

    ID3D12Device10* get_logical_device();
    IDXGIAdapter4* get_physical_adapter();


}