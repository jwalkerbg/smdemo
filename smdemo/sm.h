// $Id$

#if !defined(sm_h_included)
#define sm_h_included

#include <stdint.h>
#include <stdbool.h>
#include "sm_conf.h"

#if !defined(sizeofA)
// size of an array; should not be used with pointers
#define sizeofA(a) (sizeof(a)/sizeof((a)[0]))
#endif

extern EVENT_TYPE Event;

void SM_PutEvent (EVENT_TYPE event);
EVENT_TYPE SM_FetchEvent (void);
void SM_FlushEvents (void);

// types
typedef int STATE_TYPE;
typedef struct sm_machine SM_MACHINE;
typedef struct sm_transition SM_TRANSITION;
typedef struct sm_state SM_STATE;

typedef void (*SM_ACTION)(SM_MACHINE* this);
typedef bool (*SM_GUARD)(SM_MACHINE* this);
#if defined(SM_TRACER)
typedef void (*SM_TRANSITION_TRACER)(SM_MACHINE* this, const SM_TRANSITION* tr);
typedef void (*SM_CONTEXT_TRACER)(SM_MACHINE* this, uint8_t when);
typedef void (*SM_LOSTEVENT_TRACER)(SM_MACHINE* this);
#endif  // defined(SM_TRACER)

// transition
#define SM_GPOL_POSITIVE    (false)
#define SM_GPOL_NEGATIVE    (true)
struct sm_transition {
    EVENT_TYPE event;       // trigger event
    STATE_TYPE s2;          // destination state (index in an array)
    SM_ACTION a;            // action (can be NULL: null action)
    int ai;                 // action index
    SM_GUARD guard;         // guard: true: transition is permitted, false: transition is forbidden;
                            // (can be NULL, then transition is permitted
    bool gpol;              // guard polarity: SM_GPOL_POSITIVE or SM_GPOL_NEGATIVE
};

// state
struct sm_state {
    const SM_TRANSITION* tr;    // array of exit transitions (can be NULL: no exit transition from the state)
    int size;                   // number of transitions in tr[]
    SM_ACTION entry_action;     // action on entering state, may be NULL
    SM_ACTION exit_action;      // action on exiting state, may be NULL
};

// state machine

// masks for .flags
#define SM_ACTIVE   0b00000001u // the state machine is active (accepts events)
#define SM_TREN     0b00000010u // true: found transition is permitted by tr->guard
#define SM_TRACE    0b10000000u // tracing, if compiled, is enabled

#define SM_INVALID (-1)         // no valid responce is posibble

struct sm_machine {
    STATE_TYPE s1;              // current state fo state machine (index)
    int id;                     // state machine identifier (must be unique in the system)
    int flags;                  // internal flags see masks for .flags above
    EVENT_TYPE ev;              // incoming event been handled
    const SM_STATE* states;     // array of states, s1 is index for it
    int sizes;                  // number of states in states[]
    void* ctx;                  // pointer to struct containing context information for SM
                                // can be NULL
#if defined(SM_TRACER)
    SM_TRANSITION_TRACER trm;   // pointer to tracer of the state machine
    SM_CONTEXT_TRACER trc;      // pointer to context tracer
    SM_LOSTEVENT_TRACER trle;   // tracer of of lost events
#endif  // defined(SM_TRACER)
};

void SM_Machine (SM_MACHINE* this, EVENT_TYPE ev);

void SM_Initialize (SM_MACHINE* this, STATE_TYPE s1, int id, const SM_STATE* states, int sizes, void* ctx);

int SM_GetID (SM_MACHINE* this);
int SM_GetCurrentState (SM_MACHINE* this);
bool SM_SetCurrentState (SM_MACHINE* this, STATE_TYPE s1);
int SM_GetStateCount (SM_MACHINE* this);

void SM_SetContext (SM_MACHINE* this, void* ctx);
void* SM_GetContext (SM_MACHINE* this);

void SM_Start (SM_MACHINE* this, STATE_TYPE s1);
void SM_StartWithEvent (SM_MACHINE* this, STATE_TYPE s1, EVENT_TYPE ev);
void SM_Activate (SM_MACHINE* this);
void SM_Deactivate (SM_MACHINE* this);
bool SM_IsActivated (SM_MACHINE* this);

#if defined(SM_TRACER)

// trace data
struct sm_tracedata {
    uint32_t time;          // time when transition is executed
    int id;                 // state machine identifier
    STATE_TYPE s1;          // start state
    STATE_TYPE s2;          // target state
    EVENT_TYPE ev;          // trigger event
};
typedef struct sm_tracedata SM_TRACEDATA;

void SM_TraceOn (SM_MACHINE* this);
void SM_TraceOff (SM_MACHINE* this);
void SM_SetTracers(SM_MACHINE* this, SM_TRANSITION_TRACER trm, SM_CONTEXT_TRACER trc, SM_LOSTEVENT_TRACER trle);
bool SM_IsTraceEnabled (SM_MACHINE* this);

#endif  // defined(SM_TRACER)

#endif  // !defined(sm_h_included)

// end of sm.h
