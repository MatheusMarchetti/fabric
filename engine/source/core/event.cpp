#include "core/event.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"
#include "containers/darray.hpp"

using namespace fabric;

namespace internal {
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

    static event_code_entry* entries[max_message_codes];

}  // namespace

b8 event::initialize() {
    if (internal::is_initialized) {
        FBERROR("Event system was already initialized!");
        return false;
    }

    memory::fbzero(internal::entries, sizeof(internal::entries));

    FBINFO("Event system initialized.");

    return internal::is_initialized = true;
}

void event::terminate() {
    for (u16 i = 0; i < internal::max_message_codes; i++) {
        if (internal::entries[i] && !internal::entries[i]->events.empty()) {
            internal::entries[i]->events.clear();

            memory::fbfree(internal::entries[i], sizeof(internal::event_code_entry*), memory::MEMORY_TAG_APPLICATION);
        }
    }

    internal::is_initialized = false;
}

b8 event::checkin(u16 code, void* listener, on_event_pfn on_event) {
    if (!internal::is_initialized) {
        FBERROR("Trying to register an event before the event system was initialized.");
        return false;
    }

    if(internal::entries[code] == 0) {
        internal::entries[code] = (internal::event_code_entry*)memory::fballocate(sizeof(internal::event_code_entry*), memory::MEMORY_TAG_APPLICATION);
        internal::entries[code]->events = ftl::darray<internal::registered_event>();
    }

    u64 registered_count = internal::entries[code]->events.length();

    for (u64 i = 0; i < registered_count; i++) {
        if (internal::entries[code]->events[i].listener == listener) {
            FBWARN("Trying to register the same listener again. Nothing will be done.");
            return false;
        }
    }

    internal::registered_event event;
    event.listener = listener;
    event.callback = on_event;
    internal::entries[code]->events.push(event);

    return true;
}

b8 event::checkout(u16 code, void* listener, on_event_pfn on_event) {
    if (!internal::is_initialized) {
        FBERROR("Trying to unregister an event before the event system was initialized.");
        return false;
    }

    if (!internal::entries[code]) {
        FBWARN("Trying to unregister an event that was not registered before. Nothing will be done.");
        return false;
    }

    u64 registered_count = internal::entries[code]->events.length();
    for (u64 i = 0; i < registered_count; i++) {
        internal::registered_event& e = internal::entries[code]->events[i];
        if (e.listener == listener && e.callback == on_event) {
            internal::entries[code]->events.pop(i);
            return true;
        }
    }

    // Nothing was found
    return false;
}

b8 event::send(u16 code, void* sender, event::context eventContext) {
    if (!internal::is_initialized) {
        FBERROR("Trying to send an event before the event system was initialized.");
        return false;
    }

    if (!internal::entries[code] || internal::entries[code]->events.empty()) {
        return false;
    }

    u64 registered_count = internal::entries[code]->events.length();
    for (u64 i = 0; i < registered_count; i++) {
        internal::registered_event& e = internal::entries[code]->events[i];
        if (e.callback(code, sender, e.listener, eventContext)) {
            // Message was handled, stop dispatching
            return true;
        }
    }

    // Nothing was found
    return false;
}