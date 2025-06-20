// events.h

#pragma once

#if defined(_cplusplus)
extern "C" {
#endif

#define EVENT_LIST \
    X(evNullEvent) \
    X(evP1Start) \
    X(evP1Trigger1) X(evP1Trigger2) X(evP1Trigger3) X(evP1Trigger4) X(evP1Trigger5) \
    X(evButtonSingleClick)

    // Generate the enum automatically
typedef enum {
    #define X(name) name,
    EVENT_LIST
    #undef X
    evEVENTSNUMBER  // Total count of events
} EVENT_TYPE;

#if defined(__cplusplus)
}
#endif

// end of events.h
