#pragma once

#include "renderer/d3d12/d3d12_common.hpp"

namespace fabric {
    class d3d12_resource;
}

namespace fabric::backend {
    const d3d12_resource& create_depth_texture(u16 width, u16 height, u32 index = -1);
    const d3d12_resource& create_depth_stencil_texture(u16 width, u16 height, u32 index = -1);
    const d3d12_resource& create_render_target_texture(u16 width, u16 height, DXGI_FORMAT format, u32 index = -1);
    const d3d12_resource& create_readonly_texture2D(u16 width, u16 height, DXGI_FORMAT format, u8 mipLevels = 1, u32 index = -1);
    const d3d12_resource& create_writable_texture2D(u16 width, u16 height, DXGI_FORMAT format, u8 mipLevels = 1, u32 index = -1);

    descriptor_allocation create_render_target_view(ID3D12Resource2* resource, const D3D12_RENDER_TARGET_VIEW_DESC& desc, D3D12_CPU_DESCRIPTOR_HANDLE handle = {});
    descriptor_allocation create_depth_stencil_view(ID3D12Resource2* resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc, D3D12_CPU_DESCRIPTOR_HANDLE handle = {});
    descriptor_allocation create_readonly_texture_view(ID3D12Resource2* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, D3D12_CPU_DESCRIPTOR_HANDLE handle = {});
    descriptor_allocation create_writable_texture_view(ID3D12Resource2* resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, D3D12_CPU_DESCRIPTOR_HANDLE handle = {});
}  // namespace fabric::backend