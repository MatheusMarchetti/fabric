#include "renderer/d3d12/d3d12_device.hpp"
#include "renderer/d3d12/d3d12_descriptor_allocator.hpp"
#include "core/logger.hpp"
#include "containers/darray.hpp"

using namespace fabric;

namespace {
    IDXGIAdapter4* physical_adapter;
    ID3D12Device10* logical_device;
}  // namespace

b8 d3d12_device::create(IDXGIFactory7* dxgiFactory) {
    u8 i = 0;
    u8 best_adapter = 0;
    SIZE_T total_vram = 0;
    D3D_FEATURE_LEVEL max_supported_feature_level;
    IDXGIAdapter1* adapter;
    ID3D12Device* device;

    while (dxgiFactory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if ((desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE) == 0 &&
            desc.DedicatedVideoMemory > total_vram) {
            for (u16 feature_level = D3D_FEATURE_LEVEL_12_2; feature_level >= D3D_FEATURE_LEVEL_12_0; feature_level -= 0x0100) {
                if (SUCCEEDED(D3D12CreateDevice(adapter, (D3D_FEATURE_LEVEL)feature_level, IID_PPV_ARGS(&device)))) {
                    u8 requirements = 0;

                    D3D12_FEATURE_DATA_D3D12_OPTIONS binding_tier;
                    binding_tier.ResourceBindingTier = D3D12_RESOURCE_BINDING_TIER_3;
                    requirements = SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &binding_tier, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS))) ? (1 << 0) : 0;

                    D3D12_FEATURE_DATA_SHADER_MODEL shader_model;
                    shader_model.HighestShaderModel = D3D_SHADER_MODEL_6_6;
                    requirements = SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shader_model, sizeof(D3D12_FEATURE_SHADER_MODEL))) ? (1 << 1) : 0;

                    if (requirements != 0) {
                        max_supported_feature_level = (D3D_FEATURE_LEVEL)feature_level;
                        best_adapter = i;
                        total_vram = desc.DedicatedVideoMemory;
                    }

                    device->Release();
                    break;
                }
            }
        }

        adapter->Release();
        i++;
    }

    if (total_vram == 0) {
        FBERROR("A physical adapter with DirectX12 support (FEATURE_LEVEL_12_0 or above) is required");
        return false;
    }

    HRCheck(dxgiFactory->EnumAdapters1(best_adapter, &adapter));
    HRCheck(adapter->QueryInterface(&physical_adapter));
    adapter->Release();

    // TODO: Get adapter information

    HRESULT hr = D3D12CreateDevice(physical_adapter, max_supported_feature_level, IID_PPV_ARGS(&logical_device));

    if (FAILED(hr)) {
        FBERROR("Logical device creation failed with error %s", hr);
        return false;
    }

    NAME_OBJECTS(logical_device);

    return true;
}

void d3d12_device::destroy() {
    logical_device->Release();
    physical_adapter->Release();
}

ID3D12Device10* d3d12_device::get_logical_device() {
    return logical_device;
}

IDXGIAdapter4* get_physical_adapter() {
    return physical_adapter;
}