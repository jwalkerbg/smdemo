#include <stdio.h>
#include <stdint.h>

#include "events.h"
#include "sm.h"

enum sP1_states {
    sP1_START, sP1_RESOLVE, sP1_F1, sP1_F2, sP1_F3, sP1_F4,

    sP1_StatesNumber
};

void P1a0(SM_MACHINE* this);
void P1a1(SM_MACHINE* this);
void P1a2(SM_MACHINE* this);
void P1a3(SM_MACHINE* this);
void P1a4(SM_MACHINE* this);
void P1a5(SM_MACHINE* this);
void P1a6(SM_MACHINE* this);

bool not_permitting_guard(SM_MACHINE* this);
bool permitting_guard(SM_MACHINE* this);
void P1_F1_entry_action(SM_MACHINE* this);
void P1_F1_entry_action(SM_MACHINE* this);

enum test_actions {
    iP1a0, iP1a1, iP1a2, iP1a3, iP1a4, iP1a5, iP1a6,
    Actions_number,
    undefined_action
};

enum test_actions Action;

bool P1_F1_entry_action_executed;
bool P1_F1_exit_action_executed;
bool SM_TraceMachineExecuted;
bool SM_TraceContextExecuted;
bool SM_TraceMachineCaughtForbiddenTransition;
bool SM_TraceContextCaughtForbiddenTransition;
bool SM_TraceMachineCaughtPermittedTransition;
bool SM_TraceContextCaughtPermittedTransition;
bool SM_TraceContextExecutedFirstCall;
bool SM_TraceContextExecutedSecondCall;

bool not_permitting_guard(SM_MACHINE* this)
{
    return false;
}

bool permitting_guard(SM_MACHINE* this)
{
    return true;
}

void P1_F1_entry_action(SM_MACHINE* this)
{
    printf("\tP1_F1_entry_action executed\n");
    P1_F1_entry_action_executed = true;
}

void P1_F1_exit_action(SM_MACHINE* this)
{
    printf("\tP1_F1_exit_action executed\n");
    P1_F1_exit_action_executed = true;
}

void P1a0(SM_MACHINE* this)
{
    printf("\tiP1a0 executed\n");
    Action = iP1a0;
}

void P1a1(SM_MACHINE* this)
{
    printf("\tiP1a1 executed\n");
    Action = iP1a1;
}

void P1a2(SM_MACHINE* this)
{
    printf("\tiP1a2 executed\n");
    Action = iP1a2;
}

void P1a3(SM_MACHINE* this)
{
    printf("\tiP1a3 executed\n");
    Action = iP1a3;
}

void P1a4(SM_MACHINE* this)
{
    printf("\tiP1a4 executed\n");
    Action = iP1a4;
}

void P1a5(SM_MACHINE* this)
{
    printf("\tiP1a5 executed\n");
    Action = iP1a5;
}

void P1a6(SM_MACHINE* this)
{
    printf("\tiP1a6 executed\n");
    Action = iP1a6;
}

const SM_TRANSITION sP1_START_transitions [] = {
    { evBTN1Pressed, (STATE_TYPE)sP1_RESOLVE, P1a0, iP1a0, NULL, SM_GPOL_POSITIVE }
};

const SM_TRANSITION sP1_RESOLVE_transitions [] = {
    { evP1Trigger1, (STATE_TYPE)sP1_F1, P1a1, iP1a1, not_permitting_guard, SM_GPOL_POSITIVE },
    { evP1Trigger2, (STATE_TYPE)sP1_F2, P1a3, iP1a3, permitting_guard, SM_GPOL_POSITIVE },
    { evP1Trigger3, (STATE_TYPE)sP1_F3, P1a4, iP1a4, NULL, SM_GPOL_POSITIVE },
    { evP1Trigger4, (STATE_TYPE)sP1_F4, P1a5, iP1a5, NULL, SM_GPOL_POSITIVE },
    { evP1Trigger5, (STATE_TYPE)sP1_StatesNumber, P1a5, iP1a5, NULL, SM_GPOL_POSITIVE }
};

const SM_TRANSITION sP1_F1_transitions [] = {
    { evBTN1Released, (STATE_TYPE)sP1_START, P1a2, iP1a2, NULL, SM_GPOL_POSITIVE },
    { evBTN2Pressed, (STATE_TYPE)sP1_F2, P1a3, iP1a3, NULL, SM_GPOL_POSITIVE },
    { evBTN3Pressed, (STATE_TYPE)sP1_F3, P1a4, iP1a4, NULL, SM_GPOL_POSITIVE },
    { evP1Trigger5, (STATE_TYPE)sP1_F1, P1a6, iP1a6, NULL, SM_GPOL_POSITIVE }
};

const SM_TRANSITION sP1_F2_transitions [] = {
    { evBTN1Released, (STATE_TYPE)sP1_START, P1a2, iP1a2, NULL, SM_GPOL_POSITIVE },
    { evBTN2Released, (STATE_TYPE)sP1_F1, P1a1, iP1a1, NULL, SM_GPOL_POSITIVE },
    { evBTN3Pressed, (STATE_TYPE)sP1_F4, P1a5, iP1a5, NULL, SM_GPOL_POSITIVE }
};

const SM_TRANSITION sP1_F3_transitions [] = {
    { evBTN1Released, (STATE_TYPE)sP1_START, P1a2, iP1a2, NULL, SM_GPOL_POSITIVE },
    { evBTN3Released, (STATE_TYPE)sP1_F1, P1a1, iP1a1, NULL, SM_GPOL_POSITIVE },
    { evBTN2Pressed, (STATE_TYPE)sP1_F4, P1a5, iP1a5, NULL, SM_GPOL_POSITIVE }
};

const SM_TRANSITION sP1_F4_transitions [] = {
    { evBTN1Released, (STATE_TYPE)sP1_START, P1a2, iP1a2, NULL, SM_GPOL_POSITIVE },
    { evBTN2Released, (STATE_TYPE)sP1_F3, P1a4, iP1a4, NULL, SM_GPOL_POSITIVE },
    { evBTN3Released, (STATE_TYPE)sP1_F2, P1a3, iP1a3, NULL, SM_GPOL_POSITIVE }
};

const SM_STATE P1States [] = {
    { sP1_START_transitions, sizeofA(sP1_START_transitions), NULL, NULL },
    { sP1_RESOLVE_transitions, sizeofA(sP1_RESOLVE_transitions), NULL, NULL },
    { sP1_F1_transitions, sizeofA(sP1_F1_transitions), P1_F1_entry_action, P1_F1_exit_action },
    { sP1_F2_transitions, sizeofA(sP1_F2_transitions), NULL, NULL },
    { sP1_F3_transitions, sizeofA(sP1_F3_transitions), NULL, NULL },
    { sP1_F4_transitions, sizeofA(sP1_F4_transitions), NULL, NULL }
};

// state machine identifier
#define TEST_MACHINE_ID (1u)

SM_MACHINE test_machine;

struct test_context {
    uint16_t value;
};
typedef struct test_context TEST_CONTEXT;

#if defined(SM_TRACER)

const char* const fake_events_list_names[] = {
    "evFakeNullevent",

    "evBTN1Pressed", "evBTN1Released",
    "evBTN2Pressed", "evBTN2Released",
    "evBTN3Pressed", "evBTN3Released",
    "evP1Trigger1", "evP1Trigger2", "evP1Trigger3", "evP1Trigger4",

    "evP1Trigger5"
};

const char* const sP1_states_names[] = {
    "sP1_START", "sP1_RESOLVE", "sP1_F1", "sP1_F2", "sP1_F3", "sP1_F4"
};

// Trace order:

// 1. SM_TraceContext(sm,true)  -- information before exitting s1
// 2. s1.exit_action            -- exit action of s1
// 3. tr.action                 -- transition action
// 4. SM_TraceMachine(sm,tr)    -- trace transition
// 4. s2.entry_action           -- enter s2
// 5. SM_TraceContext(sm,false) -- information after entering s2

// void SM_TraceMachine (SM_MACHINE* this, const SM_TRANSITION* tr)
// Parameters:
//   SM_MACHINE* this - pointer to state machine
//   const SM_TRANSITION* tr - pointer to the transition being executed
// Return: no
// Description: SM_TraceMachine traces transitions of state machines. It accepts via arguments
// current transition data: this->s1, tr->s2, tr->event, tr->a.
// All actions on tracing are left to the application programmer because they are not known
// at the time of writing this module. An example of tracing is output of formatted
// string (by printf) to stdout.

// SM_TraceMachine may distiguish permitted from not permitted transition by looking
// flag SM_TREN. If SM_TREN is 1 (true), transition is permitted.

void SM_TraceMachine (SM_MACHINE* this, const SM_TRANSITION* tr)
{
    printf("ID=%04d, S1=%s, S2=%s, Event=%s, Action=%d %spermitted\n",this->id,sP1_states_names[this->s1],sP1_states_names[tr->s2],fake_events_list_names[tr->event],Action,(this->flags & SM_TREN) == 0 ? "not " : "");
    SM_TraceMachineExecuted = true;

    if ((this->flags & SM_TREN) == 0u) {
        SM_TraceMachineCaughtForbiddenTransition = true;
    }

    if ((this->flags & SM_TREN) != 0u) {
        SM_TraceMachineCaughtPermittedTransition = true;
    }
}

// void SM_TraceContext(SM_MACHINE* this, U8 when)
// Parameters:
//   SM_MACHINE* this - pointer to state machine
//   U8 when - true: before leaving s1, false: after entering s2
// Return: no
// Description: SM_TraceContext traces the context information associated to *this.
// sm->ctx, when it is not NULL points to a struct associated to the state machine instance.
// The data interpretation is up to application programmer.

// SM_TraceContext is called twice and when s1 = s2, in contrary of s1.exit_action and s2.entry_action,
// which are called when s1 != s2 only.

// If transition is not permitted (tr.guard returns false) SM_TraceContext is called before SM_TraceMachine(sm,tr):

// 1. SM_TraceContext(sm,true)  -- information for s1
// 2. SM_TraceMachine(sm,tr)    -- information for forbidden transition

// SM_TraceContext may distiguish permitted from not permitted transition by looking
// flag SM_TREN. If SM_TREN is 1 (true), transition is permitted.

void SM_TraceContext(SM_MACHINE* this, uint8_t when)
{
    SM_TraceContextExecuted = true;

    if ((this->flags & SM_TREN) == 0u) {
        SM_TraceContextCaughtForbiddenTransition = true;
    }

    if ((this->flags & SM_TREN) != 0u) {
        SM_TraceContextCaughtPermittedTransition = true;
    }

    if (when == true) {
        SM_TraceContextExecutedFirstCall = true;
    }
    if (when == false) {
        SM_TraceContextExecutedSecondCall = true;
    }
}

EVENT_TYPE IgnoredEvents[] = { evBTN3Pressed, evBTN3Released, evP1Trigger1, evP1Trigger2 };
void SM_LostEvent(SM_MACHINE* this)
{
    EVENT_TYPE ev = this->ev;
    bool found = false;
    for (int i = 0u; i < sizeofA(IgnoredEvents); i++) {
        if (IgnoredEvents[i] == ev) {
            found = true;
            break;
        }
    }
    if (found == true) {
        printf("lost: %s\n",fake_events_list_names[ev]);
    }
}

#endif  // defined(SM_TRACER)

void CatchPeripheralEvents();

void CatchPeripheralEvents()
{
    static uint8_t ix = 0u;
    static const enum events_list events[] = {
        evBTN1Pressed, evP1Trigger1,
        evBTN3Pressed, evBTN2Pressed, evBTN3Released,
        evBTN2Released, evBTN1Released,
        evP1Trigger1, evP1Trigger2, evP1Trigger4,
        evP1Trigger5, evBTN3Pressed, evBTN2Pressed, evP1Trigger1
    };

    SM_PutEvent(events[ix]);
    ix = (ix + 1) % sizeof(events);
}

TEST_CONTEXT test_ctx;

int main()
{
  printf("Hello World!\n");

  SM_Initialize (&test_machine, sP1_START, TEST_MACHINE_ID, P1States, sizeof(P1States) / sizeof(P1States[0]), NULL);
#if defined(SM_TRACER)
  SM_SetTracers(&test_machine,SM_TraceMachine, SM_TraceContext, SM_LostEvent);
#endif  // defined(SM_TRACER)
  SM_StartWithEvent(&test_machine,sP1_RESOLVE,evP1Trigger1);
#if defined(SM_TRACER)
  SM_TraceOn(&test_machine);
#endif  // defined(SM_TRACER)
  int c = 0;
  do {
      CatchPeripheralEvents();
      Event = SM_FetchEvent();
      if (Event != evNullEvent) {
          SM_Machine(&test_machine,Event);
      }
  } while (++c < 200);

  return 0;
}
