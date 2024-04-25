#pragma once

#include "defines.hpp"
#include "renderer/d3d12/d3d12_common.hpp"

#include "renderer/resources.hpp"

namespace fabric::d3d12_context {
    b8 create(IDXGIFactory7* dxgiFactory);
    void destroy();

    ID3D12Device10* get_logical_device();
    IDXGIAdapter4* get_physical_adapter();

    handle<texture> create_render_target(ID3D12Resource2* resource, D3D12_RENDER_TARGET_VIEW_DESC desc);
    handle<texture> create_depth_stencil(ID3D12Resource2* resource, D3D12_DEPTH_STENCIL_VIEW_DESC desc);
    handle<texture> create_readonly_texture(ID3D12Resource2* resource, D3D12_SHADER_RESOURCE_VIEW_DESC desc);
    handle<texture> create_writable_texture(ID3D12Resource2* resource, D3D12_UNORDERED_ACCESS_VIEW_DESC desc);

    handle<texture> create_render_target(handle<texture> texture, D3D12_RENDER_TARGET_VIEW_DESC desc);
    handle<texture> create_depth_stencil(handle<texture> texture, D3D12_DEPTH_STENCIL_VIEW_DESC desc);
    handle<texture> create_readonly_texture(handle<texture> texture, D3D12_SHADER_RESOURCE_VIEW_DESC desc);
    handle<texture> create_writable_texture(handle<texture> texture, D3D12_UNORDERED_ACCESS_VIEW_DESC desc);
}