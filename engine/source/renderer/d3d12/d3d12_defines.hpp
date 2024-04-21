#pragma once

#include "defines.hpp"
#include "core/asserts.hpp"

#define HRCheck(expr)           \
    {                           \
        FBASSERT(expr == S_OK); \
    }

#define NAME_OBJECTS(obj)    \
    {                        \
        obj->SetName(L#obj); \
    }

#define NAME_OBJECTS_TYPE(obj, type)       \
    {                                      \
        obj->SetName(L#type); \
    }