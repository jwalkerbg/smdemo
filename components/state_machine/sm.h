// sm.h

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include "esp_err.h"

// event loop definitions and declarations

#define SM_MAX_STATE_MACHINES   (CONFIG_SM_MAX_STATE_MACHINES)

#define SM_EVENT_LOOP_QUEUE_SIZE    (CONFIG_SM_EVENT_LOOP_QUEUE_SIZE)
#define SM_EVENT_TASK               ("sm_evt_task")
#define SM_EVENT_TASK_PRIORITY      (1)
#define SM_EVENT_TASK_STACK_SIZE    (CONFIG_SM_EVENT_TASK_STACK_SIZE)

#if defined(CONFIG_SM_EVENT_TYPE_DEFINED_IN_APPLICATION)
#include "events.h"
#else

// TODO: Add real events here. Do not remove evNullEvent and sm_EVENTS_NUMBER
typedef enum {
    evNullEvent,

    evEVENT1,
    evEVENT2,

    sm_EVENTS_NUMBER
} sm_event_type_t;

#endif  // !defined(CONFIG_SM_EVENT_TYPE_DEFINED_IN_APPLICATION)

esp_err_t sm_create_event_loop(void);
esp_err_t sm_post_event(sm_event_type_t event);
esp_err_t sm_post_event_with_data(sm_event_type_t event, void* event_data, size_t data_size);

// forward definition of types
typedef uint8_t sm_state_idx;
typedef struct sm_machine sm_machine_t;
typedef struct sm_transition sm_transition_t;
typedef struct sm_state sm_state_t;

typedef void (*sm_action_t)(sm_machine_t* machine);
typedef uint8_t (*sm_guard_t)(sm_machine_t* machine);
#if defined(CONFIG_SM_TRACER)
typedef void (*sm_transition_tracer_t)(sm_machine_t* machine, const sm_transition_t* tr);
typedef void (*sm_context_tracer_t)(sm_machine_t* machine, bool when);
#if defined(CONFIG_SM_TRACER_LOSTEVENT)
typedef void (*sm_lostevent_tracer_t)(sm_machine_t* machine);
#endif  // defined(CONFIG_SM_TRACER_LOSTEVENT)
#endif  // defined(CONFIG_SM_TRACER)

// transition
#define SM_GPOL_POSITIVE    (0u)
#define SM_GPOL_NEGATIVE    (1u)
struct sm_transition {
    sm_event_type_t event;  // trigger event
    sm_state_idx s2;        // destination state (index in an array)
    sm_action_t action;     // action (can be NULL: null action)
    uint8_t actidx;         // action index
    sm_guard_t guard;       // guard: true: transition is permitted, false: transition is forbidden;
                            // (can be NULL, then transition is permitted
    uint8_t gpol;           // guard polarity: SM_GPOL_POSITIVE or SM_GPOL_NEGATIVE
};

// state
struct sm_state {
    const sm_transition_t* transitions; // array of exit transitions (can be NULL: no exit transition from the state)
    uint32_t size;                      // number of transitions in tr[]
    sm_action_t entry_action;           // action on entering state, may be NULL
    sm_action_t exit_action;            // action on exiting state, may be NULL
};

// state machine

// masks for .flags
#define SM_ACTIVE   (0b00000001u)   // the state machine is active (accepts events)
#define SM_TREN     (0b00000010u)   // true: found transition is permitted by transition->guard
#define SM_TRACE    (0b10000000u)   // tracing, if compiled, is enabled
#define SM_TRACE_LE (0b01000000u)   // tracing lost events, if compiled, is enabled

#define SM_INVALID (255u)           // no valid response is possible

struct sm_machine {
    sm_state_idx s1;            // current state of state machine (index)
    uint8_t id;                 // state machine identifier (must be unique in the system)
    uint8_t flags;              // internal flags see masks for .flags above
    sm_event_type_t event;      // active event been handled
    void* event_data;           // pointer to event data (can be NULL)
    const sm_state_t* states;   // array of states, s1 is index for it
    uint32_t sizes;             // number of states in states[]
    void* ctx;                  // pointer to struct containing context information for SM
                                // can be NULL
#if defined(CONFIG_SM_TRACER)
    sm_transition_tracer_t trm;   // pointer to tracer of the state machine (transition tracer)
    sm_context_tracer_t trc;      // pointer to context tracer
#if defined(CONFIG_SM_TRACER_LOSTEVENT)
    sm_lostevent_tracer_t trle;   // tracer of of lost events
#endif  // defined(CONFIG_SM_TRACER_LOSTEVENT)
#endif  // defined(CONFIG_SM_TRACER)
};

esp_err_t sm_register_state_machine(sm_machine_t* machine);

void sm_machine(sm_machine_t* machine, sm_event_type_t ev, void* event_data);

void sm_initialize(sm_machine_t* machine, sm_state_idx s1, uint8_t id, const sm_state_t* states, uint32_t sizes, void* ctx);

uint8_t sm_get_id(sm_machine_t* machine);
uint8_t sm_get_current_state(sm_machine_t* machine);
uint8_t sm_set_current_state(sm_machine_t* machine, sm_state_idx s1);
uint8_t sm_get_state_count(sm_machine_t* machine);

void sm_set_context(sm_machine_t* machine, void* ctx);
void* sm_get_context(sm_machine_t* machine);

void sm_start(sm_machine_t* machine, sm_state_idx s1);
void sm_start_with_event(sm_machine_t* machine, sm_state_idx s1, sm_event_type_t ev);
void sm_activate(sm_machine_t* machine);
void sm_deactivate(sm_machine_t* machine);
bool sm_is_activated(sm_machine_t* machine);

#if defined(CONFIG_SM_TRACER)

// trace data
struct sm_tracedata {
    uint32_t time;          // time when transition is executed
    uint8_t id;             // state machine identifier
    sm_state_idx s1;        // start state
    sm_state_idx s2;        // target state
    sm_event_type_t ev;     // trigger event
};
typedef struct sm_tracedata SM_TRACEDATA;

void sm_trace_on(sm_machine_t* machine);
void sm_trace_off(sm_machine_t* machine);
void sm_trace_lost_event_on(sm_machine_t* machine);
void sm_trace_lost_event_off(sm_machine_t* machine);
void sm_set_tracers(sm_machine_t* machine, sm_transition_tracer_t trm, sm_context_tracer_t trc, sm_lostevent_tracer_t trle);
bool sm_is_trace_enabled(sm_machine_t* machine);

#endif  // defined(CONFIG_SM_TRACER)

#ifdef __cplusplus
}
#endif

// end of sm.h
