#pragma once

#include "defines.hpp"

namespace fabric::event {
    struct context {
        union data {
            i64 i64[2];
            u64 u64[2];
            f64 f64[2];

            i32 i32[4];
            u32 u32[4];
            f32 f32[4];

            i16 i16[8];
            u16 u16[8];

            i8 i8[16];
            u8 u8[16];

            char c[16];
        };
    };

    // The first 255 codes are reserved for internal use.
    enum system_code : u16 {
        // Tells the application to quit on the next frame.
        APPLICATION_QUIT = 0x01,

        // Keyboard key pressed.
        /* Context usage:
         * u16 key_code = data.u16[0];
         */
        KEY_PRESSED = 0x02,

        // Keyboard key released.
        /* Context usage:
         * u16 key_code = data.u16[0];
         */
        KEY_RELEASED = 0x03,

        // Mouse button pressed.
        /* Context usage:
         * u16 button = data.u16[0];
         */
        BUTTON_PRESSED = 0x04,

        // Mouse button released.
        /* Context usage:
         * u16 button = data.u16[0];
         */
        BUTTON_RELEASED = 0x05,

        // Mouse moved.
        /* Context usage:
         * u16 x = data.u16[0];
         * u16 y = data.u16[1];
         */
        MOUSE_MOVED = 0x06,

        // Mouse wheel.
        /* Context usage:
         * u8 z_delta = data.u8[0];
         */
        MOUSE_WHEEL = 0x07,

        // Resized/resolution changed.
        /* Context usage:
         * u16 width = data.u16[0];
         * u16 height = data.u16[1];
         */
        RESIZED = 0x08,

        MAX_CODES = 0xFF
    };

    // Returns true if event was handled
    typedef b8 (*on_event_pfn)(u16 code, void* sender, void* listenerInst, fabric::event::context data);

    b8 initialize();
    void terminate();

    FB_API b8 checkin(u16 code, void* listener, on_event_pfn on_event);
    FB_API b8 checkout(u16 code, void* listener, on_event_pfn on_event);
    FB_API b8 send(u16 code, void* sender, context eventContext);

}