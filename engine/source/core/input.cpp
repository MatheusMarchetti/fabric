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

    struct system_state {
        keyboard keyboard_current;
        keyboard keyboard_previous;
        mouse mouse_current;
        mouse mouse_previous;
    };

    static system_state* state;
}  // namespace

b8 input::initialize(u64& memory_requirement, void* memory) {
    memory_requirement = sizeof(system_state);
    if(!memory) {
        return true;
    }

    if (state) {
        FBERROR("Input system was already initialized!");
        return false;
    }
    state = (system_state*)memory;
    memory::fbzero(state, sizeof(system_state));

    FBINFO("Input system initialized.");

    return true;
}

void input::terminate() {
    state = nullptr;
}

void input::update(f64 timestep) {
    if (state) {
        memory::fbcopy(&state->keyboard_previous, &state->keyboard_current, sizeof(keyboard));
        memory::fbcopy(&state->mouse_previous, &state->mouse_current, sizeof(mouse));
    }
}

void input::process_key(keys key, b8 pressed) {
    if (state->keyboard_current.keys[key] != pressed) {
        state->keyboard_current.keys[key] = pressed;

        event::context context;
        context.data.u16[0] = key;

        event::send(pressed ? event::KEY_PRESSED : event::KEY_RELEASED, nullptr, context);
    }
}

void input::process_button(buttons button, b8 pressed) {
    if (state->mouse_current.buttons[button] != pressed) {
        state->mouse_current.buttons[button] = pressed;

        event::context context;
        context.data.u16[0] = button;

        event::send(pressed ? event::BUTTON_PRESSED : event::BUTTON_RELEASED, nullptr, context);
    }
}

void input::process_mouse_move(i16 x, i16 y) {
    if (state->mouse_current.x != x || state->mouse_current.y != y) {
        state->mouse_current.x = x;
        state->mouse_current.y = y;

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
    if (!state) {
        FBERROR("Trying to query for inputs before the input system was initialized.");
        return false;
    }

    return state->keyboard_current.keys[key] == true;
}

b8 input::is_key_up(keys key) {
    if (!state) {
        FBERROR("Trying to query for inputs before the input system was initialized.");
        return false;
    }

    return state->keyboard_current.keys[key] == false;
}

b8 input::was_key_down(keys key) {
    if (!state) {
        FBERROR("Trying to query for inputs before the input system was initialized.");
        return false;
    }

    return state->keyboard_previous.keys[key] == true;
}

b8 input::was_key_up(keys key) {
    if (!state) {
        FBERROR("Trying to query for inputs before the input system was initialized.");
        return false;
    }

    return state->keyboard_previous.keys[key] == false;
}

b8 input::is_button_down(buttons button) {
    if (!state) {
        FBERROR("Trying to query for inputs before the input system was initialized.");
        return false;
    }

    return state->mouse_current.buttons[button] == true;
}

b8 input::is_button_up(buttons button) {
    if (!state) {
        FBERROR("Trying to query for inputs before the input system was initialized.");
        return false;
    }

    return state->mouse_current.buttons[button] == false;
}

b8 input::was_button_down(buttons button) {
    if (!state) {
        FBERROR("Trying to query for inputs before the input system was initialized.");
        return false;
    }

    return state->mouse_previous.buttons[button] == true;
}

b8 input::was_button_up(buttons button) {
    if (!state) {
        FBERROR("Trying to query for inputs before the input system was initialized.");
        return false;
    }

    return state->mouse_previous.buttons[button] == false;
}

void input::get_mouse_position(i32& x, i32& y) {
    if (!state) {
        FBERROR("Trying to query for inputs before the input system was initialized.");
        x = y = 0;
        return;
    }

    x = state->mouse_current.x;
    y = state->mouse_current.y;
}

void input::get_previous_mouse_position(i32& x, i32& y) {
    if (!state) {
        FBERROR("Trying to query for inputs before the input system was initialized.");
        x = y = 0;
        return;
    }

    x = state->mouse_previous.x;
    y = state->mouse_previous.y;
}
