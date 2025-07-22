// events.h

#pragma once

#if defined(_cplusplus)
extern "C" {
#endif

#define EVENT_LIST \
    X(evNullEvent) \
    X(evP1Start) \
    X(evP1Trigger1) X(evP1Trigger2) X(evP1Trigger3) X(evP1Trigger4) X(evP1Trigger5) \
    X(evButtonSingleClick) \
    X(ev_t_blink_changer_tick) \

// Generate the enum automatically
typedef enum {
    #define X(name) name,
    EVENT_LIST
    #undef X
    sm_EVENTS_NUMBER  // Total count of events
} sm_event_type_t;

#if defined(__cplusplus)
}
#endif

// end of events.h
