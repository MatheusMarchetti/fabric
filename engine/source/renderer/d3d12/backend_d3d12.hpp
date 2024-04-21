#pragma once

#include "defines.hpp"

extern "C" {
b8 d3d12_initialize(void* state);
void d3d12_terminate();
void d3d12_begin_frame();
void d3d12_end_frame();
void d3d12_resize(u16 width, u16 height);
void d3d12_update(f64 timestep);
b8 d3d12_present();
}