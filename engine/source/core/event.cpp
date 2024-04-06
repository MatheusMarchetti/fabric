#include "core/event.hpp"
#include "core/logger.hpp"
#include "core/memory.hpp"
#include "containers/darray.hpp"

using namespace fabric;

namespace {
    struct registered_event {
        void* listener;
        event::on_event_pfn callback;
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
    if (is_initialized) {
        FBERROR("Event system was already initialized!");
        return false;
    }

    memory::fbzero(entries, sizeof(event_code_entry) * max_message_codes);

    return is_initialized = true;
}

void event::terminate() {
    for (u16 i = 0; i < max_message_codes; i++) {
        if (entries[i] && !entries[i]->events.empty()) {
            entries[i]->events.clear();

            memory::fbfree(entries[i], sizeof(event_code_entry), memory::MEMORY_TAG_APPLICATION);
        }
    }
}

b8 event::checkin(u16 code, void* listener, on_event_pfn on_event) {
    if (!is_initialized) {
        FBERROR("Trying to register an event before the event system was initialized.");
        return false;
    }

    if(entries[code] == 0) {
        entries[code] = (event_code_entry*)memory::fballocate(sizeof(event_code_entry), memory::MEMORY_TAG_APPLICATION);
    }

    u64 registered_count = entries[code]->events.length();

    for (u64 i = 0; i < registered_count; i++) {
        if (entries[code]->events[i].listener == listener) {
            FBWARN("Trying to register the same listener again. Nothing will be done.");
            return false;
        }
    }

    registered_event event;
    event.listener = listener;
    event.callback = on_event;
    entries[code]->events.push(event);

    return true;
}

b8 event::checkout(u16 code, void* listener, event::on_event_pfn on_event) {
    if (!is_initialized) {
        FBERROR("Trying to unregister an event before the event system was initialized.");
        return false;
    }

    if (!entries[code]) {
        FBWARN("Trying to unregister an event that was not registered before. Nothing will be done.");
        return false;
    }

    u64 registered_count = entries[code]->events.length();
    for (u64 i = 0; i < registered_count; i++) {
        registered_event e = entries[code]->events[i];
        if (e.listener == listener&& e.callback == on_event) {
            entries[code]->events.pop(i);
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

    if (!entries[code] || entries[code]->events.empty()) {
        FBWARN("No events matching this code were found. Nothing will be done.");
        return false;
    }

    u64 registered_count = entries[code]->events.length();
    for (u64 i = 0; i < registered_count; i++) {
        registered_event e = entries[code]->events[i];
        if (e.callback(code, sender, e.listener, eventContext)) {
            // Message was handled, stop dispatching
            return true;
        }
    }

    // Nothing was found
    return false;
}