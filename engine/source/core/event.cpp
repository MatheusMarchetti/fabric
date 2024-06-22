#include "core/event.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"
#include "ftl/darray.hpp"

using namespace fabric;

namespace {
    // If this isn't enough...
    static constexpr u16 max_message_codes = 16384U;

    struct registered_event {
        void* listener;
        on_event_pfn callback;
    };

    struct event_code_entry {
        ftl::darray<registered_event> events;
    };

    struct system_state {
        event_code_entry entries[max_message_codes];
    };

    static system_state* state;

}  // namespace

b8 event::initialize(u64& memory_requirement, void* memory) {
    memory_requirement = sizeof(system_state);
    if(!memory) {
        return true;
    }

    if (state) {
        FBERROR("Event system was already initialized!");
        return false;
    }
    state = (system_state*)memory;
    memory::fbzero(state, sizeof(system_state));

    FBINFO("Event system initialized.");

    return true;
}

void event::terminate() {
    state = nullptr;
}

b8 event::checkin(u16 code, void* listener, on_event_pfn on_event) {
    if (!state) {
        FBERROR("Trying to register an event before the event system was initialized.");
        return false;
    }

    if(state->entries[code].events.capacity() == 0) {
        state->entries[code].events.reserve(1);
    }

    u64 registered_count = state->entries[code].events.length();

    for (u64 i = 0; i < registered_count; i++) {
        if (state->entries[code].events[i].listener == listener) {
            FBWARN("Trying to register the same listener again. Nothing will be done.");
            return false;
        }
    }

    registered_event event;
    event.listener = listener;
    event.callback = on_event;
    state->entries[code].events.push(event);

    return true;
}

b8 event::checkout(u16 code, void* listener, on_event_pfn on_event) {
    if (!state) {
        FBERROR("Trying to unregister an event before the event system was initialized.");
        return false;
    }

    if (state->entries[code].events.empty()) {
        FBWARN("Trying to unregister an event that was not registered before. Nothing will be done.");
        return false;
    }

    u64 registered_count = state->entries[code].events.length();
    for (u64 i = 0; i < registered_count; i++) {
        registered_event& e = state->entries[code].events[i];
        if (e.listener == listener && e.callback == on_event) {
            state->entries[code].events.pop(i);
            return true;
        }
    }

    // Nothing was found
    return false;
}

b8 event::send(u16 code, void* sender, event::context eventContext) {
    if (!state) {
        FBERROR("Trying to send an event before the event system was initialized.");
        return false;
    }

    if (state->entries[code].events.empty()) {
        return false;
    }

    u64 registered_count = state->entries[code].events.length();
    for (u64 i = 0; i < registered_count; i++) {
        registered_event& e = state->entries[code].events[i];
        if (e.callback(code, sender, e.listener, eventContext)) {
            // Message was handled, stop dispatching
            return true;
        }
    }

    // Nothing was found
    return false;
}