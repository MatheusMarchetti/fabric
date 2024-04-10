#include "core/input.hpp"
#include "core/event.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"

using namespace fabric;

namespace {
    struct keyboard {
        b8 keys[input::keys::KEYS_MAX_COUNT];
    };

    struct mouse {
        i16 x;
        i16 y;
        b8 buttons[input::buttons::BUTTON_MAX_COUNT];
    };

    keyboard keyboard_current;
    keyboard keyboard_previous;
    mouse mouse_current;
    mouse mouse_previous;

    b8 is_initialized = false;
}  // namespace

b8 input::initialize() {
    if (is_initialized) {
        FBERROR("Input system was already initialized!");
        return false;
    }

    memory::fbzero(&keyboard_current, sizeof(keyboard));
    memory::fbzero(&keyboard_previous, sizeof(keyboard));
    memory::fbzero(&mouse_current, sizeof(mouse));
    memory::fbzero(&mouse_previous, sizeof(mouse));

    FBINFO("Input system initialized.");

    return is_initialized = true;
}

void input::terminate() {
    is_initialized = false;
}

void input::update(f64 timestep) {
    if (is_initialized) {
        memory::fbcopy(&keyboard_previous, &keyboard_current, sizeof(keyboard));
        memory::fbcopy(&mouse_previous, &mouse_current, sizeof(mouse));
    }
}

void input::process_key(keys key, b8 pressed) {
    if (keyboard_current.keys[key] != pressed) {
        keyboard_current.keys[key] = pressed;

        event::context context;
        context.data.u16[0] = key;

        event::send(pressed ? event::KEY_PRESSED : event::KEY_RELEASED, nullptr, context);
    }
}

void input::process_button(buttons button, b8 pressed) {
    if (mouse_current.buttons[button] != pressed) {
        mouse_current.buttons[button] = pressed;

        event::context context;
        context.data.u16[0] = button;

        event::send(pressed ? event::BUTTON_PRESSED : event::BUTTON_RELEASED, nullptr, context);
    }
}

void input::process_mouse_move(i16 x, i16 y) {
    if (mouse_current.x != x || mouse_current.y != y) {
        mouse_current.x = x;
        mouse_current.y = y;

        event::context context;
        context.data.u16[0] = x;
        context.data.u16[1] = y;

        event::send(event::MOUSE_MOVED, nullptr, context);
    }
}

void input::process_mouse_wheel(i8 z_delta) {
    event::context context;
    context.data.u8[0] = z_delta;

    event::send(event::MOUSE_WHEEL, nullptr, context);
}

b8 input::is_key_down(keys key) {
    if (!is_initialized) {
        FBERROR("Trying to query for inputs before the input system was initialized.");
        return false;
    }

    return keyboard_current.keys[key] == true;
}

b8 input::is_key_up(keys key) {
    if (!is_initialized) {
        FBERROR("Trying to query for inputs before the input system was initialized.");
        return false;
    }

    return keyboard_current.keys[key] == false;
}

b8 input::was_key_down(keys key) {
    if (!is_initialized) {
        FBERROR("Trying to query for inputs before the input system was initialized.");
        return false;
    }

    return keyboard_previous.keys[key] == true;
}

b8 input::was_key_up(keys key) {
    if (!is_initialized) {
        FBERROR("Trying to query for inputs before the input system was initialized.");
        return false;
    }

    return keyboard_previous.keys[key] == false;
}

b8 input::is_button_down(buttons button) {
    if (!is_initialized) {
        FBERROR("Trying to query for inputs before the input system was initialized.");
        return false;
    }

    return mouse_current.buttons[button] == true;
}

b8 input::is_button_up(buttons button) {
    if (!is_initialized) {
        FBERROR("Trying to query for inputs before the input system was initialized.");
        return false;
    }

    return mouse_current.buttons[button] == false;
}

b8 input::was_button_down(buttons button) {
    if (!is_initialized) {
        FBERROR("Trying to query for inputs before the input system was initialized.");
        return false;
    }

    return mouse_previous.buttons[button] == true;
}

b8 input::was_button_up(buttons button) {
    if (!is_initialized) {
        FBERROR("Trying to query for inputs before the input system was initialized.");
        return false;
    }

    return mouse_previous.buttons[button] == false;
}

void input::get_mouse_position(i32& x, i32& y) {
    if (!is_initialized) {
        FBERROR("Trying to query for inputs before the input system was initialized.");
        x = y = 0;
        return;
    }

    x = mouse_current.x;
    y = mouse_current.y;
}

void input::get_previous_mouse_position(i32& x, i32& y) {
    if (!is_initialized) {
        FBERROR("Trying to query for inputs before the input system was initialized.");
        x = y = 0;
        return;
    }

    x = mouse_previous.x;
    y = mouse_previous.y;
}
