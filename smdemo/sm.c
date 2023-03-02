// $Id$

#include <stdlib.h>
#include <stdbool.h>
#include "events.h"
#include "sm.h"

EVENT_TYPE SM_EventBuffer [SM_EVENT_BUFFER_LENGTH];
uint8_t SM_EventBufferIndex;

EVENT_TYPE Event;

// void SM_PutEvent (EVENT_TYPE event)
// Parameters:
//   EVENT_TYPE event - event to be but into SM_EventBuffer[].
// Description: SM_PutEvent puts event into SM_EventBuffer[]. If there is no space in SM_EventBuffer[]
// for events this functions does nothing.

void SM_PutEvent (EVENT_TYPE event)
{
    if (SM_EventBufferIndex < SM_EVENT_BUFFER_LENGTH) {
        SM_EventBuffer[SM_EventBufferIndex++] = event;
    }
}

// EVENT_TYPE SM_FetchEvent (void)
// Parameters:
//   no
// Return:
//   Event or evNullEvent
// Description: SM_FetchEvent returns most ancient event that is in SM_EventBuffer[] or if
// the buffer is empty, returns evNullEvent.

EVENT_TYPE SM_FetchEvent (void)
{
    uint8_t j;
    EVENT_TYPE ev;

    if (SM_EventBufferIndex > 0u) {
        ev = SM_EventBuffer[0];
        SM_EventBufferIndex--;
        for (j = 0u; j < SM_EventBufferIndex; j++) {
            SM_EventBuffer[j] = SM_EventBuffer[j + 1u];
        }
        return ev;
    } else {
        return evNullEvent;
    }
}

// void SM_FlushEvents (void)
// Parameters:
//   no
// Return: no
// Description: This functions flushes all events from SM_EventBuffer[]. This way it resets the buffer.

void SM_FlushEvents (void)
{
    SM_EventBufferIndex = 0u;
}

// state machine

// void SM_machine (SM_MACHINE* this)
// Parameters:
//   SM_MACHINE* this - pointer to state machine
//   EVENT_TYPE ev - event passed ti]o themachine
// Return: no
// Description: SM_machine searches for a transition to be triggered by event ev.
// If it finds transition, it checks if it is permitted by guard (if any). If the
// transition is found and permitted, it is executed. If there is not permitted transition
// that could be triggered by ev SM_Machine does nothing.
// Note: If tracing is enabled, it is executed and for found but not permitted transitions.
//
// Order of execution:
//
// Permitted transition:
// 1. Bit SM_TREN in this->flags is set up, because permitted transition is found
// 2. SM_TraceContext(sm,true)  -- information before exiting s1
// 3. s1.exit_action            -- exit action of s1 if s1 != s2
// 4. tr.action                 -- transition action
// 5. SM_TraceMachine(sm,tr)    -- trace transition
// 6. s2.entry_action           -- entry action of s2 if s1 != s2
// 7. SM_TraceContext(sm,false) -- information after entering s2
// 8. Bit SM_TREN in this->flags is cleared
// 9. exit
//
// Forbidden transition:
// 1. SM_TraceContext(sm,true)  -- information for s1
// 2. SM_TraceMachine(sm,tr)    -- information for forbidden transition

// SM_TraceContext may distinguish permitted from not permitted transition by looking
// flag SM_TREN. If SM_TREN is 1 (true), transition is permitted.

void SM_Machine (SM_MACHINE* this, EVENT_TYPE ev)
{
    uint8_t i;                  // loop variable
    const SM_STATE* st;         // pointer to current state
    const SM_TRANSITION* tr;    // pointer to transition array of current state
    uint8_t sz;                 // number of transitions in tr[]
    static bool s1neqs2;        // true, if start and target states are different
    static bool found;          // found a transition, disabled or enabled

    if ((ev != evNullEvent) &&
        (this != NULL) &&
        ((this->flags & SM_ACTIVE) != 0u) &&
        (this->s1 < this->sizes)
       ) {
        this->ev = ev;
// copy to local variables for faster execution
        st = &(this->states[this->s1]); // current state
        if (st != NULL) {
            tr = st->tr;            // transition array of current state (or NULL)
            sz = st->size;          // number of transitions in tr[]
        } else {
            tr = NULL;              // found a NULL pointer in SM_STATE[this->s1]
            sz = 0u;
        }

// search for permitted transition that is triggered by ev
        found = false;
        if (tr != NULL) {
            for (i = 0u; i < sz; i++, tr++) {
                if (tr->event == ev) {
                    found = true;   // used by lost event tracer; otherwise ignored
// check target state for validity -- if invalid s2 is found, break SM loop and silently exit
                    if (tr->s2 >= this->sizes) {
                        break;
                    }
// check guard:
// transition is executed if (1) there is no guard or (2) the guard permits it (returns true)
                    if ((tr->guard == NULL) || ((tr->guard(this) ^ tr->gpol) == true)) {
                        this->flags |= SM_TREN;
#if defined(SM_TRACER)
                        if (SM_IsTraceEnabled(this) == true) {
                            if (this->trc != NULL) {
                                this->trc(this,true);
                            }
                        }
#endif  // defined(SM_TRACER)
// check if target state is different from start state
                        s1neqs2 = true;
                        if (tr->s2 == this->s1) {
                            s1neqs2 = false;
                        }
// call of exit_action of s1, if there is a state change and if exit_action is defined for s1
                        if (s1neqs2 == true) {
                            if (st->exit_action != NULL) {
                                st->exit_action(this);
                            }
                        }
// action on transition
                        if (tr->a != NULL) {
                            tr->a(this);
                        }
#if defined(SM_TRACER)
                        if (SM_IsTraceEnabled(this) == true) {
                            if (this->trm != NULL) {
                                this->trm(this,tr);
                            }
                        }
#endif  // defined(SM_TRACER)
// transition to s2
                        this->s1 = tr->s2;
// call of entry_action, if there is a state change and if entry_action is defined for s2
                        if (s1neqs2 == true) {
                            st = &(this->states[this->s1]);
                            if (st->entry_action != NULL) {
                                 st->entry_action(this);
                            }
                        }
#if defined(SM_TRACER)
                        if (SM_IsTraceEnabled(this) == true) {
                            if (this->trc != NULL) {
                                this->trc(this,false);
                            }
                        }
#endif  // defined(SM_TRACER)
// transition is executed, so exit from loop
                        this->flags &= ~SM_TREN;
                        break;
                    } else {
// forbidden transition
                        this->flags &= ~SM_TREN;
#if defined(SM_TRACER)
                        if (SM_IsTraceEnabled(this) == true) {
                            if (this->trc != NULL) {
                                this->trc(this,true);
                            }
                            if (this->trm != NULL) {
                                this->trm(this,tr);
                            }
                        }
#endif  // defined(SM_TRACER)
                    }
                }
            }
        }
#if defined(SM_TRACER)
        if (found == false) {
            if (SM_IsTraceEnabled(this) == true) {
                if (this->trle != NULL) {
                    this->trle(this);
                }
            }
        }
#endif  // defined(SM_TRACER)
    }
}

// static void SM_ClearFlags(SM_MACHINE* this)
// Parameters:
//  SM_MACHINE* this - pointer to state machine
// Return: no
// Description: Reset all flags to zero (false).

void SM_ClearFlags(SM_MACHINE* this);
void SM_ClearFlags(SM_MACHINE* this)
{
    if (this != NULL) {
        this->flags = 0u;
    }
}

// void SM_Initialize (SM_MACHINE* this, STATE_TYPE s1, int id, const SM_STATE* states, int sizes, void* ctx)
// Parameters:
//   SM_MACHINE* this - pointer to state machine
//   STATE_TYPE s1 - start (initial) state
//   int id - identifier of state machine
//   const SM_STATE* states - pointer to states array
//   int sizes - size of states array
//   void* ctx - pointer to user defined context structure
// Return: no
// Description: SM_Initialize initializes *this fields with parameters.

void SM_Initialize (SM_MACHINE* this, STATE_TYPE s1, int id, const SM_STATE* states, int sizes, void* ctx)
{
    if (this != NULL) {
        this->s1 = s1;
        this->id = id;
        this->states = states;
        this->sizes = sizes;

        SM_SetContext(this,ctx);
        SM_ClearFlags(this);

#if defined(SM_TRACER)
        this->trm = NULL;
        this->trc = NULL;
        this->trle = NULL;
#endif  // defined(SM_TRACER)
    }
}

// int SM_GetID (SM_MACHINE* this)
// Parameters:
//   SM_MACHINE* this - pointer to state machine
// Return:
//   Identifier of state machine
// Description: SM_GetID returns the identifier of state machine *this.

int SM_GetID (SM_MACHINE* this)
{
    return (this != NULL) ? this->id : SM_INVALID;
}

// STATE_TYPE SM_GetCurrentState (SM_MACHINE* this)
// Parameters:
//   SM_MACHINE* this - pointer to state machine
// Return:
//   current state of *this
// Description: SM_GetCurrentState returns current state of the machine.

STATE_TYPE SM_GetCurrentState (SM_MACHINE* this)
{
    return (this != NULL) ? this->s1 : SM_INVALID;
}

// bool SM_SetCurrentState (SM_MACHINE* this, STATE_TYPE s1)
// Parameters:
//   SM_MACHINE* this - pointer to state machine
//   STATE_TYPE s1 - state
// Return:
//   true: State Machine is put in s1 state
//   false: State Machine is not touched, because s1 is invalid

bool SM_SetCurrentState (SM_MACHINE* this, STATE_TYPE s1)
{
    if (this != NULL) {
        if (s1 < this->sizes) {
            this->s1 = s1;
            return true;
        }
    }
    return false;
}

// int SM_GetStateCount (SM_MACHINE* this)
// Parameters:
//   SM_MACHINE* this - pointer to state machine
// Return:
//   number of states in *this
// Description: SM_GetStateCount returns the number of states of *this.

int SM_GetStateCount (SM_MACHINE* this)
{
    return (this != NULL) ? this->sizes : SM_INVALID;
}

// void SM_SetContext (SM_MACHINE* this, void* ctx)
// Parameters:
//   SM_MACHINE* this - pointer to state machine
// Return: no
// Description: Sets context of the state machine pointed to by *this.

void SM_SetContext (SM_MACHINE* this, void* ctx)
{
    if (this != NULL) {
        this->ctx = ctx;
    }
}

// void* SM_GetContext (SM_MACHINE* this)
// Parameters:
//   SM_MACHINE* this - pointer to state machine
// Return: pointer to context structure
// Description: Return pointer to a structure that contains context information of *this state machine.

void* SM_GetContext (SM_MACHINE* this)
{
    return (this != NULL) ? this->ctx : NULL;
}

// uint8_t SM_Start (SM_MACHINE* this, STATE_TYPE s1)
// Parameters:
//   SM_MACHINE* this - pointer to state machine
//   STATE_TYPE s1 - start state
// Return: no
// Description: SM_Start puts the state machine pointed to by *this in state s1 and activates it.
// If s1 is not valid this function does not have any effect. It the state machine has been already
// activated, this function does not do anything.
// After call of SM_StartWithEvent the caller should check if *this is activated.

void SM_Start (SM_MACHINE* this, STATE_TYPE s1)
{
    if (this != NULL) {
        if (SM_IsActivated(this) == false) {
            if (SM_SetCurrentState(this,s1) == true) {
                SM_Activate(this);
            }
        }
    }
}

// void SM_StartWithEvent (SM_MACHINE* this, STATE_TYPE s1, EVENT_TYPE ev)
// Parameters:
//   SM_MACHINE* this - pointer to state machine
//   STATE_TYPE s1 - start state
//   EVENT_TYPE ev - event that triggers *this in s1
// Return: no
// Description: SM_StartWithEvent puts the state machine pointed to by *this in state s1, activates it
// and then injects event ev. If s1 is not valid this function does not have any effect. If the state machine
// has been already activated, this function does not do anything.
// After the call of SM_StartWithEvent the caller should check if *this is activated.

void SM_StartWithEvent (SM_MACHINE* this, STATE_TYPE s1, EVENT_TYPE ev)
{
    if (this != NULL) {
        if (SM_IsActivated(this) == false) {
            if (ev < evEventsNumber) {
                if (SM_SetCurrentState(this,s1) == true) {
                    SM_Activate(this);
                    SM_PutEvent(ev);
                }
            }
        }
    }
}

// void SM_Activate (SM_MACHINE* this)
// Parameters:
//   SM_MACHINE* this - pointer to state machine
// Return: no
// Description: SM_Activate activates *this.

void SM_Activate (SM_MACHINE* this)
{
    if (this != NULL) {
        this->flags |= SM_ACTIVE;
    }
}

// void SM_Deactivate (SM_MACHINE* this)
// Parameters:
//   SM_MACHINE* this - pointer to state machine
// Return: no
// Description: SM_Deactivate deactivates *this. *this will not act on any events.

void SM_Deactivate (SM_MACHINE* this)
{
    if (this != NULL) {
        this->flags &= ~SM_ACTIVE;
    }
}

// bool SM_IsActivated (SM_MACHINE* this)
// Parameters:
//   SM_MACHINE* this - pointer to state machine
// Return: true: *this is active; false: *this is not active
// Description: Determine if *this is active.

bool SM_IsActivated (SM_MACHINE* this)
{
    if (this != NULL) {
        return ((this->flags & SM_ACTIVE) != 0u) ? true : false;
    }
    return false;
}

#if defined(SM_TRACER)

// void SM_TraceOn (SM_MACHINE* this)
// Parameters:
//   SM_MACHINE* this - pointer to state machine
// Return: no
// Description: SM_TraceOn enables tracing of *this state machine

void SM_TraceOn (SM_MACHINE* this)
{
    if (this != NULL) {
        this->flags |= SM_TRACE;
    }
}

// void SM_TraceOff (SM_MACHINE* this)
// Parameters:
//   SM_MACHINE* this - pointer to state machine
// Return: no
// Description: SM_TraceOff disables tracing of *this state machine

void SM_TraceOff (SM_MACHINE* this)
{
    if (this != NULL) {
        this->flags &= ~SM_TRACE;
    }
}

// void SM_SetTracers(SM_MACHINE* this, SM_TRANSITION_TRACER trm, SM_CONTEXT_TRACER trc
// Parameters:
//  SM_MACHINE* this - pointer to state machine
//  SM_TRANSITION_TRACER trm - pointer to a transition tracer
//  SM_CONTEXT_TRACER trc - pointer to a context tracer
//  SM_LOSTEVENT_TRACER trle - pointer to a lost events tracer
void SM_SetTracers(SM_MACHINE* this, SM_TRANSITION_TRACER trm, SM_CONTEXT_TRACER trc, SM_LOSTEVENT_TRACER trle)
{
    if (this != NULL) {
        this->trm = trm;
        this->trc = trc;
        this->trle = trle;
    }
}

// bool SM_IsTraceEnabled (SM_MACHINE* this)
// Parameters:
//   SM_MACHINE* this - pointer to state machine
// Return: true: trace is enabled; false: trace is disabled
// Description: Determine if tracing of *this is enabled.

bool SM_IsTraceEnabled (SM_MACHINE* this)
{
    if (this != NULL) {
        return ((this->flags & SM_TRACE) != 0) ? true : false;
    }
    return false;
}

#endif  // defined(SM_TRACER)

// end of sm.c
