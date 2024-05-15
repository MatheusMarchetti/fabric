#pragma once

#include "defines.hpp"
#include "renderer/d3d12/d3d12_common.hpp"
#include "renderer/resources.hpp"

extern "C" {
b8 d3d12_initialize(void* state);
void d3d12_terminate();
void d3d12_begin_frame();
void d3d12_end_frame();
void d3d12_resize(u16 width, u16 height);
void d3d12_update(f64 timestep);
b8 d3d12_present();
}

namespace fabric {
    class d3d12_descriptor_allocator;
}

namespace fabric::backend {
    struct resource {
        ID3D12Resource2* resource;
        D3D12_RESOURCE_STATES current_state;
    };

    d3d12_descriptor_allocator& get_descriptor_allocator(D3D12_DESCRIPTOR_HEAP_TYPE type);
    resource& get_resource(u32 index);
    u32 register_resource(ID3D12Resource2* resource, D3D12_RESOURCE_STATES state);

    handle<texture> create_render_target(ID3D12Resource2* texture, D3D12_RENDER_TARGET_VIEW_DESC desc);
    handle<texture> create_render_target(handle<texture> texture, D3D12_RENDER_TARGET_VIEW_DESC desc, ID3D12Resource2* resource = nullptr);
    handle<texture> create_depth_stencil(handle<texture> texture, D3D12_DEPTH_STENCIL_VIEW_DESC desc);
    handle<texture> create_readonly_texture(handle<texture> texture, D3D12_SHADER_RESOURCE_VIEW_DESC desc);
    handle<texture> create_writable_texture(handle<texture> texture, D3D12_UNORDERED_ACCESS_VIEW_DESC desc);
}  // namespace fabric::backend