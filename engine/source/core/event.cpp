#include "core/event.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"
#include "containers/darray.hpp"

using namespace fabric;

namespace {
    struct registered_event {
        void* listener;
        on_event_pfn callback;
    };

    struct event_code_entry {
        ftl::darray<registered_event> events;
    };

    // If this isn't enough...
    static constexpr u16 max_message_codes = 16384U;

    b8 is_initialized = false;

    static event_code_entry entries[max_message_codes];

}  // namespace

b8 event::initialize() {
    if (is_initialized) {
        FBERROR("Event system was already initialized!");
        return false;
    }

    // memory::fbzero(entries, sizeof(entries));

    FBINFO("Event system initialized.");

    return is_initialized = true;
}

void event::terminate() {
    is_initialized = false;
}

b8 event::checkin(u16 code, void* listener, on_event_pfn on_event) {
    if (!is_initialized) {
        FBERROR("Trying to register an event before the event system was initialized.");
        return false;
    }

    if(entries[code].events.capacity() == 0) {
        entries[code].events.reserve(1);
    }

    u64 registered_count = entries[code].events.length();

    for (u64 i = 0; i < registered_count; i++) {
        if (entries[code].events[i].listener == listener) {
            FBWARN("Trying to register the same listener again. Nothing will be done.");
            return false;
        }
    }

    registered_event event;
    event.listener = listener;
    event.callback = on_event;
    entries[code].events.push(event);

    return true;
}

b8 event::checkout(u16 code, void* listener, on_event_pfn on_event) {
    if (!is_initialized) {
        FBERROR("Trying to unregister an event before the event system was initialized.");
        return false;
    }

    if (entries[code].events.empty()) {
        FBWARN("Trying to unregister an event that was not registered before. Nothing will be done.");
        return false;
    }

    u64 registered_count = entries[code].events.length();
    for (u64 i = 0; i < registered_count; i++) {
        registered_event& e = entries[code].events[i];
        if (e.listener == listener && e.callback == on_event) {
            entries[code].events.pop(i);
            return true;
        }
    }

    // Nothing was found
    return false;
}

b8 event::send(u16 code, void* sender, event::context eventContext) {
    if (!is_initialized) {
        FBERROR("Trying to send an event before the event system was initialized.");
        return false;
    }

    if (entries[code].events.empty()) {
        return false;
    }

    u64 registered_count = entries[code].events.length();
    for (u64 i = 0; i < registered_count; i++) {
        registered_event& e = entries[code].events[i];
        if (e.callback(code, sender, e.listener, eventContext)) {
            // Message was handled, stop dispatching
            return true;
        }
    }

    // Nothing was found
    return false;
}