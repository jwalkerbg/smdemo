
#include <stdbool.h>
#include <stdint.h>
#include "esp_attr.h"
#include <string.h>
#include <esp_log.h>

#include "commondefs.h"
#include "proc.h"
#include "process.h"

static const char TAG[] = "PROCESS";

#if defined(CONFIG_SM_TRACER)
void SM_TraceContext(SM_MACHINE* machine, uint8_t when);
#endif  // defined(CONFIG_SM_TRACER)

// sm_P1 Main process ================================================

enum action_ids_P1 {
    iP1a0 = 0, iP1a1, iP1a2, iP1a3, iP1a4, iP1a5, iP1a6, iP1a7, iP1a8, iP1a9, iP1a10, iP1a11
};

static void P1a0(SM_MACHINE* machine);
static void P1a1(SM_MACHINE* machine);
static void P1a2(SM_MACHINE* machine);
static void P1a3(SM_MACHINE* machine);
static void P1a4(SM_MACHINE* machine);

static void P1a0(SM_MACHINE* machine)
{
    ESP_LOGI(TAG,"P1a0 executed");

    read_opmode();
    device_modes_t ops = get_opmode();

    switch (ops) {
        case OP_MODE_IDLE:
            sm_post_event(evP1Trigger1);
            break;
        case OP_MODE_AUTO:
            sm_post_event(evP1Trigger2);
            break;
        case OP_MODE_NIGHT:
            sm_post_event(evP1Trigger3);
            break;
        case OP_MODE_MANUAL:
            sm_post_event(evP1Trigger4);
            break;
        default:
            break;
    }
}

// going to sP1_STANDBY
static void P1a1(SM_MACHINE* machine)
{
    ESP_LOGI(TAG,"P1a1 executed");
    sm_post_event(evP1Trigger3);
}

// going to sP1_AUTO
static void P1a2(SM_MACHINE* machine)
{
    ESP_LOGI(TAG,"P1a2 executed");
}

// going to sP1_MANUAL
static void P1a3(SM_MACHINE* machine)
{
    ESP_LOGI(TAG,"P1a3 executed");
}

// going to sP1_TEST
static void P1a4(SM_MACHINE* machine)
{
    ESP_LOGI(TAG,"P1a4 executed");
}

const SM_TRANSITION sP1_START_transitions[] = {
    { evP1Start, (STATE_TYPE)sP1_RESOLVE, P1a0, iP1a0, NULL, SM_GPOL_POSITIVE },
};

const SM_TRANSITION sP1_RESOLVE_transitions[] = {
    { evP1Trigger1, (STATE_TYPE)sP1_STANDBY, P1a1, iP1a1, NULL, SM_GPOL_POSITIVE },
    { evP1Trigger2, (STATE_TYPE)sP1_AUTO, P1a2, iP1a2, NULL, SM_GPOL_POSITIVE },
    { evP1Trigger3, (STATE_TYPE)sP1_MANUAL, P1a3, iP1a3, NULL, SM_GPOL_POSITIVE },
    { evP1Trigger4, (STATE_TYPE)sP1_MANUAL, P1a4, iP1a4, NULL, SM_GPOL_POSITIVE },
};

const SM_TRANSITION sP1_STANDBY_transitions[] = {
    { 0 }
};

const SM_TRANSITION sP1_AUTO_transitions[] = {
    { 0 }
};

const SM_TRANSITION sP1_MANUAL_transitions[] = {
    { 0 }
};

const SM_TRANSITION sP1_TEST_transitions[] = {
    { 0 }
};

// sm_P1 state machine definition
const SM_STATE P1_States[sP1_STATE_COUNT] = {
    { sP1_START_transitions, ARRAY_SIZE(sP1_START_transitions), NULL, NULL },
    { sP1_RESOLVE_transitions, ARRAY_SIZE(sP1_RESOLVE_transitions), NULL, NULL},
    { sP1_STANDBY_transitions, ARRAY_SIZE(sP1_STANDBY_transitions), NULL, NULL},
    { sP1_AUTO_transitions, ARRAY_SIZE(sP1_AUTO_transitions), NULL, NULL},
    { sP1_MANUAL_transitions, ARRAY_SIZE(sP1_MANUAL_transitions), NULL, NULL},
    { sP1_TEST_transitions, ARRAY_SIZE(sP1_TEST_transitions), NULL, NULL},
};


SM_MACHINE sm_P1 = { 0 };
static P1_context_t P1_ctx = { 0 };
#if defined(CONFIG_SM_TRACER)
static void SM_TraceMachine_1 (SM_MACHINE* machine, const SM_TRANSITION* tr);
static void SM_LostEvent_1(SM_MACHINE* machine);
#endif  // defined(SM_TRACER)

esp_err_t register_state_machines(void)
{
    esp_err_t ret = ESP_OK;

    ret = SM_register_state_machine(&sm_P1);


    return ret == ESP_OK ? ESP_OK : ESP_FAIL;
}

void P1_start(void)
{
    if (SM_IsActivated(&sm_P1) != 0) {
        return;
    }

    SM_Initialize(&sm_P1, sP1_START, P1_ID, P1_States, ARRAY_SIZE(P1_States),&P1_ctx);
    SM_SetTracers(&sm_P1,SM_TraceMachine_1, SM_TraceContext, SM_LostEvent_1);
    SM_TraceOn(&sm_P1);
    SM_TraceLostEventOn(&sm_P1);
    SM_StartWithEvent(&sm_P1,sP1_START,evP1Start);
}

void P1_stop(void)
{
    // stop any resources running related to P1
    SM_Deactivate(&sm_P1);
}

// tracers

#if defined(CONFIG_SM_TRACER)

// Generate the event_names array automatically
const char* const event_names[] = {
    #define X(name) #name,
    EVENT_LIST
    #undef X
};

// Array of state names. When adding new states, add their names in the same order.

const char* const sP1_state_names[] = {
    #define X(name) #name,
    P1_STATES
    #undef X
};

// Trace order:

// 1. SM_TraceContext(sm,false) -- information before exitting s1
// 2. s1.exit_action            -- exit action of s1
// 3. tr.action                 -- transition action
// 4. SM_TraceMachine(sm,tr)    -- trace transition
// 5. s2.entry_action           -- enter s2
// 6. SM_TraceContext(sm,true)  -- information after entering s2

// void SM_TraceMachine (SM_MACHINE* machine, const SM_TRANSITION* tr)
// Parameters:
//   SM_MACHINE* machine - pointer to state machine
//   const SM_TRANSITION* tr - pointer to the transition being executed
//   const char** state_names - array of state names
// Return: no
// Description: SM_TraceMachine traces transitions of state machines. It accepts via arguments
// current transition data: machine->s1, tr->s2, tr->event, tr->a.
// All actions on tracing are left to the application programmer because they are not known
// at the time of writing machine module. An example of tracing is output of formatted
// string (by printf) to stdout.

// SM_TraceMachine may distiguish permitted from not permitted transition by looking
// flag SM_TREN. If SM_TREN is 1 (true), transition is permitted.

static void SM_TraceMachine_ (SM_MACHINE* machine, const SM_TRANSITION* tr, const char* const * state_names)
{
    ESP_LOGI(TAG,"ID=%04d, S1=%s, S2=%s, Event=%s, Action=%d %spermitted",
        machine->id,state_names[machine->s1],state_names[tr->s2],event_names[tr->event],tr->ai,(machine->flags & SM_TREN) == 0 ? "not " : "");
}

static void SM_TraceMachine_1 (SM_MACHINE* machine, const SM_TRANSITION* tr)
{
    SM_TraceMachine_(machine,tr,sP1_state_names);
}

// void SM_TraceContext(SM_MACHINE* machine, U8 when)
// Parameters:
//   SM_MACHINE* machine - pointer to state machine
//   U8 when - true: before leaving s1, false: after entering s2
// Return: no
// Description: SM_TraceContext traces the context information associated to *machine.
// sm->ctx, when is not NULL points to a struct associated to the state machine instance.
// The data interpretation is up to application programmer.

// SM_TraceContext is called twice and when s1 = s2, in contrary of s1.exit_action and s2.entry_action,
// which are called when s1 != s2 only.

// If transition is not permitted (tr.guard returns false) SM_TraceContext is called before SM_TraceMachine(sm,tr):

// 1. SM_TraceContext(sm,true)  -- information for s1
// 2. SM_TraceMachine(sm,tr)    -- information for forbidden transition

// SM_TraceContext may distinguish permitted from not permitted transition by looking
// flag SM_TREN. If SM_TREN is 1 (true), transition is permitted.

void SM_TraceContext(SM_MACHINE* machine, uint8_t when)
{
    ESP_LOGI(TAG,"Trace context %d",when);
}

static void SM_LostEvent_(SM_MACHINE* machine, const char* const * state_names)
{
    EVENT_TYPE ev = machine->ev;

    if (ev < evEVENTSNUMBER) {
        ESP_LOGI(TAG,"ID=%04d: Lost ev: %s, state: %s",machine->id,event_names[ev],state_names[machine->s1]);
    }
}

static void SM_LostEvent_1(SM_MACHINE* machine)
{
    SM_LostEvent_(machine,sP1_state_names);
}

#endif  // defined(CONFIG_SM_TRACER)