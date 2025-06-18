// sm.h

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
//#include "esp_event.h"

// event loop definitions and declarations

#if CONFIG_SM_EVENT_TYPE_DEFINED_IN_APPLICATION==1
#define SM_EVENT_TYPE_DEFINED_IN_APPLICATION
#endif

#define SM_MAX_STATE_MACHINES   (CONFIG_SM_MAX_STATE_MACHINES)

#define SM_EVENT_LOOP_QUEUE_SIZE    (CONFIG_SM_EVENT_LOOP_QUEUE_SIZE)
#define SM_EVENT_TASK               ("sm_evt_task")
#define SM_EVENT_TASK_PRIORITY      (1)
#define SM_EVENT_TASK_STACK_SIZE    (CONFIG_SM_EVENT_TASK_STACK_SIZE)

#if defined(SM_EVENT_TYPE_DEFINED_IN_APPLICATION)
#include "events.h"
#else

// TODO: Add real events here. Do not remove evNullEvent and evEVENTSNUMBER
typedef enum {
    evNullEvent,

    evEVENT1,
    evEVENT2,

    evEVENTSNUMBER
} EVENT_TYPE;

#endif  // !defined(SM_EVENT_TYPE_DEFINED_IN_APPLICATION)

esp_err_t sm_create_event_loop(void);
esp_err_t sm_post_event(EVENT_TYPE event);
esp_err_t sm_post_event_with_data(EVENT_TYPE event, void* event_data, size_t data_size);

// types
typedef uint8_t STATE_TYPE;
typedef struct sm_machine SM_MACHINE;
typedef struct sm_transition SM_TRANSITION;
typedef struct sm_state SM_STATE;

typedef void (*SM_ACTION)(SM_MACHINE* machine);
typedef uint8_t (*SM_GUARD)(SM_MACHINE* machine);
#if defined(CONFIG_SM_TRACER)
typedef void (*SM_TRANSITION_TRACER)(SM_MACHINE* machine, const SM_TRANSITION* tr);
typedef void (*SM_CONTEXT_TRACER)(SM_MACHINE* machine, uint8_t when);
#if defined(CONFIG_SM_TRACER_LOSTEVENT)
typedef void (*SM_LOSTEVENT_TRACER)(SM_MACHINE* machine);
#endif  // defined(CONFIG_SM_TRACER_LOSTEVENT)
#endif  // defined(CONFIG_SM_TRACER)

// transition
#define SM_GPOL_POSITIVE    (0u)
#define SM_GPOL_NEGATIVE    (1u)
struct sm_transition {
    EVENT_TYPE event;       // trigger event
    STATE_TYPE s2;          // destination state (index in an array)
    SM_ACTION action;       // action (can be NULL: null action)
    uint8_t actidx;         // action index
    SM_GUARD guard;         // guard: true: transition is permitted, false: transition is forbidden;
                            // (can be NULL, then transition is permitted
    uint8_t gpol;           // guard polarity: SM_GPOL_POSITIVE or SM_GPOL_NEGATIVE
};

// state
struct sm_state {
    const SM_TRANSITION* transitions;   // array of exit transitions (can be NULL: no exit transition from the state)
    uint32_t size;              // number of transitions in tr[]
    SM_ACTION entry_action;     // action on entering state, may be NULL
    SM_ACTION exit_action;      // action on exiting state, may be NULL
};

// state machine

// masks for .flags
#define SM_ACTIVE   (0b00000001u)   // the state machine is active (accepts events)
#define SM_TREN     (0b00000010u)   // true: found transition is permitted by transition->guard
#define SM_TRACE    (0b10000000u)   // tracing, if compiled, is enabled
#define SM_TRACE_LE (0b01000000u)   // tracing lost events, if compiled, is enabled

#define SM_INVALID (255u)       // no valid responce is posibble

struct sm_machine {
    STATE_TYPE s1;              // current state fo state machine (index)
    uint8_t id;                 // state machine identifier (must be unique in the system)
    uint8_t flags;              // internal flags see masks for .flags above
    EVENT_TYPE event;           // active event been handled
    void* event_data;           // pointer to event data (can be NULL)
    const SM_STATE* states;     // array of states, s1 is index for it
    uint32_t sizes;             // number of states in states[]
    void* ctx;                  // pointer to struct containing context information for SM
                                // can be NULL
#if defined(CONFIG_SM_TRACER)
    SM_TRANSITION_TRACER trm;   // pointer to tracer of the state machine
    SM_CONTEXT_TRACER trc;      // pointer to context tracer
#if defined(CONFIG_SM_TRACER_LOSTEVENT)
    SM_LOSTEVENT_TRACER trle;   // tracer of of lost events
#endif  // defined(CONFIG_SM_TRACER_LOSTEVENT)
#endif  // defined(CONFIG_SM_TRACER)
};

esp_err_t SM_register_state_machine(SM_MACHINE* machine);

void SM_Machine(SM_MACHINE* machine, EVENT_TYPE ev, void* event_data);

void SM_Initialize(SM_MACHINE* machine, STATE_TYPE s1, uint8_t id, const SM_STATE* states, uint32_t sizes, void* ctx);

uint8_t SM_GetID(SM_MACHINE* machine);
uint8_t SM_GetCurrentState(SM_MACHINE* machine);
uint8_t SM_SetCurrentState(SM_MACHINE* machine, STATE_TYPE s1);
uint8_t SM_GetStateCount(SM_MACHINE* machine);

void SM_SetContext(SM_MACHINE* machine, void* ctx);
void* SM_GetContext(SM_MACHINE* machine);

void SM_Start(SM_MACHINE* machine, STATE_TYPE s1);
void SM_StartWithEvent(SM_MACHINE* machine, STATE_TYPE s1, EVENT_TYPE ev);
void SM_Activate(SM_MACHINE* machine);
void SM_Deactivate(SM_MACHINE* machine);
uint8_t SM_IsActivated(SM_MACHINE* machine);

#if defined(CONFIG_SM_TRACER)

// trace data
struct sm_tracedata {
    uint32_t time;          // time when transition is executed
    uint8_t id;             // state machine identifier
    STATE_TYPE s1;          // start state
    STATE_TYPE s2;          // target state
    EVENT_TYPE ev;          // trigger event
};
typedef struct sm_tracedata SM_TRACEDATA;

void SM_TraceOn(SM_MACHINE* machine);
void SM_TraceOff(SM_MACHINE* machine);
void SM_TraceLostEventOn(SM_MACHINE* machine);
void SM_TraceLostEventOff(SM_MACHINE* machine);
void SM_SetTracers(SM_MACHINE* machine, SM_TRANSITION_TRACER trm, SM_CONTEXT_TRACER trc, SM_LOSTEVENT_TRACER trle);
uint8_t SM_IsTraceEnabled(SM_MACHINE* machine);

#endif  // defined(CONFIG_SM_TRACER)

#ifdef __cplusplus
}
#endif

// end of sm.h
