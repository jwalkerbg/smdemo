#include "include/events.h"
#include "sdkconfig.h"

#include <stdbool.h>
#include <stdint.h>
#include "esp_attr.h"
#include <string.h>
#include <esp_log.h>

#include "commondefs.h"
#include "proc.h"
#include "process.h"
#include "anvs.h"

static const char TAG[] = "PS";

#if defined(CONFIG_SM_TRACER)
void sm_trace_context(sm_machine_t* machine, bool when);
#endif  // defined(CONFIG_SM_TRACER)

// sm_P1 Main process ================================================

enum action_ids_P1 {
    iP1a0 = 0, iP1a1, iP1a2, iP1a3, iP1a4, iP1a5, iP1a6, iP1a7, iP1a8, iP1a9, iP1a10
};

static void P1a0(sm_machine_t* machine);
static void P1a1(sm_machine_t* machine);
static void P1a2(sm_machine_t* machine);
static void P1a3(sm_machine_t* machine);
static void P1a4(sm_machine_t* machine);
static void P1a5(sm_machine_t* machine);
static void P1a6(sm_machine_t* machine);
static void P1a7(sm_machine_t* machine);
static void P1a8(sm_machine_t* machine);
static void P1a9(sm_machine_t* machine);
static void P1a10(sm_machine_t* machine);

static void P1a0(sm_machine_t* machine)
{
    ESP_LOGI(TAG,"P1a0 executed");

    read_opmode();
    device_modes_t ops = get_opmode();

    switch (ops) {
        case OP_MODE_STANDBY:
            sm_post_event(evP1Trigger1);
            break;
        case OP_MODE_AUTO:
            sm_post_event(evP1Trigger2);
            break;
        case OP_MODE_AUTO_NIGHT:
            sm_post_event(evP1Trigger3);
            break;
        case OP_MODE_MANUAL:
            sm_post_event(evP1Trigger4);
            break;
        case OP_MODE_TEST:
            sm_post_event(evP1Trigger5);
            break;
        default:
            break;
    }
}

// going to sP1_STANDBY
static void P1a1(sm_machine_t* machine)
{
    ESP_LOGI(TAG,"P1a1 executed");
    set_blink_period(0);  // Set blink period to 10Hz
}

// going to sP1_AUTO
static void P1a2(sm_machine_t* machine)
{
    ESP_LOGI(TAG,"P1a2 executed");
    set_blink_period(1);  // Set blink period to 2Hz
}

// going to sP1_AUTO_NIGHT
static void P1a3(sm_machine_t* machine)
{
    ESP_LOGI(TAG,"P1a3 executed");
    set_blink_period(2);  // Set blink period to 1Hz
}

// going to sP1_MANUAL
static void P1a4(sm_machine_t* machine)
{
    ESP_LOGI(TAG,"P1a4 executed");
    set_blink_period(3);  // Set blink period to 0.5Hz
}

// going to sP1_TEST
static void P1a5(sm_machine_t* machine)
{
    ESP_LOGI(TAG,"P1a5 executed");
    set_blink_period(4);  // Set blink period to 0.4Hz
}

static void P1a6(sm_machine_t* machine)
{
    ESP_LOGI(TAG,"P1a6 executed");
    set_blink_period(0);  // Set blink period to 10Hz
    set_opmode(OP_MODE_STANDBY);
    anvs_app_op_mode_set(OP_MODE_STANDBY);
}

static void P1a7(sm_machine_t* machine)
{
    ESP_LOGI(TAG,"P1a7 executed");
    set_blink_period(1);  // Set blink period to 2Hz
    set_opmode(OP_MODE_AUTO);
    anvs_app_op_mode_set(OP_MODE_AUTO);

}

static void P1a8(sm_machine_t* machine)
{
    ESP_LOGI(TAG,"P1a8 executed");
    set_blink_period(2);  // Set blink period to 1Hz
    set_opmode(OP_MODE_AUTO_NIGHT);
    anvs_app_op_mode_set(OP_MODE_AUTO_NIGHT);
}

static void P1a9(sm_machine_t* machine)
{
    ESP_LOGI(TAG,"P1a9 executed");
    set_blink_period(3);  // Set blink period to 0.5Hz
    set_opmode(OP_MODE_MANUAL);
    anvs_app_op_mode_set(OP_MODE_MANUAL);
}

static void P1a10(sm_machine_t* machine)
{
    ESP_LOGI(TAG,"P1a10 executed");
    set_blink_period(4);  // Set blink period to 0.4Hz
    set_opmode(OP_MODE_TEST);
    anvs_app_op_mode_set(OP_MODE_TEST);
}

static const sm_transition_t sP1_START_transitions[] = {
    { evP1Start, (sm_state_idx)sP1_RESOLVE, P1a0, iP1a0, NULL, SM_GPOL_POSITIVE },
};

static const sm_transition_t sP1_RESOLVE_transitions[] = {
    { evP1Trigger1, (sm_state_idx)sP1_STANDBY, P1a1, iP1a1, NULL, SM_GPOL_POSITIVE },
    { evP1Trigger2, (sm_state_idx)sP1_AUTO, P1a2, iP1a2, NULL, SM_GPOL_POSITIVE },
    { evP1Trigger3, (sm_state_idx)sP1_AUTO_NIGHT, P1a3, iP1a3, NULL, SM_GPOL_POSITIVE },
    { evP1Trigger4, (sm_state_idx)sP1_MANUAL, P1a4, iP1a4, NULL, SM_GPOL_POSITIVE },
    { evP1Trigger5, (sm_state_idx)sP1_MANUAL, P1a5, iP1a5, NULL, SM_GPOL_POSITIVE },
};

static const sm_transition_t sP1_STANDBY_transitions[] = {
    { evButtonSingleClick, (sm_state_idx)sP1_AUTO, P1a7, iP1a7, NULL, SM_GPOL_POSITIVE },
};

static const sm_transition_t sP1_AUTO_transitions[] = {
    { evButtonSingleClick, (sm_state_idx)sP1_AUTO_NIGHT, P1a8, iP1a8, NULL, SM_GPOL_POSITIVE },
};

static const sm_transition_t sP1_AUTO_NIGHT_transitions[] = {
    { evButtonSingleClick, (sm_state_idx)sP1_MANUAL, P1a9, iP1a9, NULL, SM_GPOL_POSITIVE },
};

static const sm_transition_t sP1_MANUAL_transitions[] = {
    { evButtonSingleClick, (sm_state_idx)sP1_TEST, P1a10, iP1a10, NULL, SM_GPOL_POSITIVE },
};

static const sm_transition_t sP1_TEST_transitions[] = {
    { evButtonSingleClick, (sm_state_idx)sP1_STANDBY, P1a6, iP1a6, NULL, SM_GPOL_POSITIVE },
};

// sm_P1 state machine definition
static const sm_state_t P1_States[sP1_STATE_COUNT] = {
    { sP1_START_transitions, ARRAY_SIZE(sP1_START_transitions), NULL, NULL },
    { sP1_RESOLVE_transitions, ARRAY_SIZE(sP1_RESOLVE_transitions), NULL, NULL},
    { sP1_STANDBY_transitions, ARRAY_SIZE(sP1_STANDBY_transitions), NULL, NULL},
    { sP1_AUTO_transitions, ARRAY_SIZE(sP1_AUTO_transitions), NULL, NULL},
    { sP1_AUTO_NIGHT_transitions, ARRAY_SIZE(sP1_AUTO_NIGHT_transitions), NULL, NULL},
    { sP1_MANUAL_transitions, ARRAY_SIZE(sP1_MANUAL_transitions), NULL, NULL},
    { sP1_TEST_transitions, ARRAY_SIZE(sP1_TEST_transitions), NULL, NULL},
};


sm_machine_t sm_P1 = { 0 };
static P1_context_t P1_ctx = { 0 };
#if defined(CONFIG_SM_TRACER)
static void sm_trace_machine_1 (sm_machine_t* machine, const sm_transition_t* tr);
static void sm_lost_event_1(sm_machine_t* machine);
#endif  // defined(SM_TRACER)

esp_err_t register_state_machines(void)
{
    esp_err_t ret = ESP_OK;

    ret = sm_register_state_machine(&sm_P1);


    return ret == ESP_OK ? ESP_OK : ESP_FAIL;
}

void P1_start(void)
{
    if (sm_is_activated(&sm_P1) != 0) {
        return;
    }

    ESP_LOGI(TAG,"Starting P1");

    sm_initialize(&sm_P1, sP1_START, P1_ID, P1_States, ARRAY_SIZE(P1_States),&P1_ctx);
    sm_set_tracers(&sm_P1,sm_trace_machine_1, sm_trace_context, sm_lost_event_1);
    sm_trace_on(&sm_P1);
    sm_trace_lost_event_on(&sm_P1);
    sm_start_with_event(&sm_P1,sP1_START,evP1Start);
}

void P1_stop(void)
{
    // stop any resources running related to P1
    sm_deactivate(&sm_P1);
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

// void SM_TraceMachine (sm_machine_t* machine, const sm_transition_t* tr)
// Parameters:
//   sm_machine_t* machine - pointer to state machine
//   const sm_transition_t* tr - pointer to the transition being executed
//   const char** state_names - array of state names
// Return: no
// Description: SM_TraceMachine traces transitions of state machines. It accepts via arguments
// current transition data: machine->s1, tr->s2, tr->event, tr->a.
// All actions on tracing are left to the application programmer because they are not known
// at the time of writing machine module. An example of tracing is output of formatted
// string (by printf) to stdout.

// SM_TraceMachine may distiguish permitted from not permitted transition by looking
// flag SM_TREN. If SM_TREN is 1 (true), transition is permitted.

static void SM_TraceMachine_ (sm_machine_t* machine, const sm_transition_t* tr, const char* const * state_names)
{
    ESP_LOGI(TAG,"ID=%04d, S1=%s, S2=%s, Event=%s, Action=%d %spermitted",
        machine->id,state_names[machine->s1],state_names[tr->s2],event_names[tr->event],tr->actidx,(machine->flags & SM_TREN) == 0 ? "not " : "");
}

static void sm_trace_machine_1 (sm_machine_t* machine, const sm_transition_t* tr)
{
    SM_TraceMachine_(machine,tr,sP1_state_names);
}

// void SM_TraceContext(sm_machine_t* machine, U8 when)
// Parameters:
//   sm_machine_t* machine - pointer to state machine
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

void sm_trace_context(sm_machine_t* machine, bool when)
{
    ESP_LOGI(TAG,"Trace context %d",when);
}

static void sm_lost_event_(sm_machine_t* machine, const char* const * state_names)
{
    sm_event_type_t ev = machine->event;

    if (ev < sm_EVENTS_NUMBER) {
        ESP_LOGI(TAG,"ID=%04d: Lost ev: %s, state: %s",machine->id,event_names[ev],state_names[machine->s1]);
    }
    else {
        ESP_LOGW(TAG,"ID=%04d: Unknown lost event with ID %d, state: %s",machine->id,ev,state_names[machine->s1]);
    }
}

static void sm_lost_event_1(sm_machine_t* machine)
{
    sm_lost_event_(machine,sP1_state_names);
}

#endif  // defined(CONFIG_SM_TRACER)