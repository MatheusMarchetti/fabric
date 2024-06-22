#pragma once

#include "defines.hpp"

struct application_config {
    const char* name;
    i16 posX;
    i16 posY;
    i16 client_width;
    i16 client_height;
};

struct application {
    application_config config;
    b8 (*initialize)() = nullptr;
    b8 (*begin_frame)(f64 timestep) = nullptr;
    b8 (*end_frame)(f64 timestep) = nullptr;
    void (*terminate)() = nullptr;
    void* internal_state = nullptr;
};