// $Id$

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"

#include "state_machine.h"

static const char SM_TAG[] = "SM";

static esp_event_loop_args_t loop_args = {
    .queue_size = SM_EVENT_LOOP_QUEUE_SIZE,
    .task_name = SM_EVENT_TASK,
    .task_priority = SM_EVENT_TASK_PRIORITY,
    .task_stack_size = SM_EVENT_TASK_STACK_SIZE
};

ESP_EVENT_DEFINE_BASE(SM_EVENTS);

esp_event_loop_handle_t sm_loop_handle;
static void sm_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void sm_machine(sm_machine_t* machine, sm_event_type_t event, void* event_data);

esp_err_t sm_create_event_loop(void)
{
    esp_err_t rtn;

    // create dedicated event loop for sm events
    rtn = esp_event_loop_create(&loop_args,&sm_loop_handle);
    if (rtn != ESP_OK) {
        ESP_LOGI(SM_TAG,"SM event loop was not created, reason: %d",rtn);
        return rtn;
    }
    ESP_LOGI(SM_TAG,"Created SM event loop");

    // register handler for commands incoming from BLE
    rtn = esp_event_handler_instance_register_with(sm_loop_handle,SM_EVENTS,ESP_EVENT_ANY_ID,sm_event_handler,NULL,NULL);
    if (rtn != ESP_OK) {
        ESP_LOGI(SM_TAG,"SM event handler for events was not registered, error: %d",rtn);
        return rtn;
    }

    ESP_LOGI(SM_TAG,"SM event loop registered");
    return rtn;
}

static uint32_t registered_machines = 0u;
static sm_machine_t* sm_list[SM_MAX_STATE_MACHINES] = { 0 };

esp_err_t sm_register_state_machine(sm_machine_t* machine)
{
    if (machine == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (registered_machines >= SM_MAX_STATE_MACHINES) {
        return ESP_ERR_NO_MEM;
    }
    sm_list[registered_machines++] = machine;
    return ESP_OK;
}

/**
 * @brief Event handler that dispatches events to all registered state machines.
 *
 * This function is called by the ESP event loop system when an event under the `SM_EVENTS`
 * base is received. It forwards the received `event_id` to all registered state machines,
 * invoking their event processing routines.
 *
 * @note Only the `event_id` and `event_data` are used. The other arguments are ignored.
 * Events with ID `evNullEvent` are ignored and not propagated to state machines.
 *
 * @param[in] event_handler_arg Unused.
 * @param[in] event_base        Unused.
 * @param[in] event_id          The event identifier to pass to the state machines.
 * @param[in] event_data        Pointer to any associated event data (may be NULL).
 *
 * @return None.
 *
 * @example
 * When an event such as `evTemperatureHigh` is posted to the SM event loop,
 * this handler forwards it to all registered state machines:
 * @code
 * esp_event_post_to(sm_loop_handle, SM_EVENTS, evTemperatureHigh, NULL, 0, portMAX_DELAY);
 * @endcode
 */
static void sm_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_id != evNullEvent) {
        // Send events to state machines.
        for (uint32_t i = 0; i < registered_machines; i++) {
            sm_machine(sm_list[i],event_id, event_data);
        }
    }
}

esp_err_t sm_post_event_with_data(sm_event_type_t event, void* event_data, size_t data_size)
{
    if (NULL != sm_loop_handle) {
        return esp_event_post_to(sm_loop_handle,SM_EVENTS,event,event_data,data_size,portMAX_DELAY);
    }
    else {
        return ESP_ERR_INVALID_ARG;
    }
}

esp_err_t sm_post_event(sm_event_type_t event)
{
    return sm_post_event_with_data(event,NULL,0u);
}

/**
 * @brief Executes the core logic of the state machine for a given event.
 *
 * This function evaluates transitions for the currently active state of a given state machine
 * and processes an incoming event. If a transition matching the event is found and permitted
 * (either has no guard or its guard evaluates as permitted), the transition is executed.
 * Otherwise, the function does nothing (or optionally logs the forbidden transition if tracing is enabled).
 *
 * ### Transition Execution Workflow:
 *
 * **Permitted transition:**
 * 1. `SM_TREN` flag is set in `machine->flags`.
 * 2. `SM_TraceContext(machine, false)` logs context before exiting current state.
 * 3. If source and target states differ, calls `exit_action` of source state (`s1`).
 * 4. Calls `action` associated with the transition.
 * 5. Calls `SM_TraceMachine(machine, transition)` to trace the transition.
 * 6. If states differ, calls `entry_action` of target state (`s2`).
 * 7. Calls `SM_TraceContext(machine, true)` after entering target state.
 * 8. Clears the `SM_TREN` flag.
 *
 * **Forbidden transition (guard returns false):**
 * 1. Calls `SM_TraceContext(machine, true)` to log state.
 * 2. Calls `SM_TraceMachine(machine, transition)` to log denied transition.
 *
 * **Lost event tracing** (if enabled with `CONFIG_SM_TRACER_LOSTEVENT`):
 * - If no transition matched the event, and `SM_TRACE_LE` flag is set,
 *   calls `machine->trle()` to trace the lost event.
 *
 * @param[in,out] machine Pointer to the state machine instance to execute.
 * @param[in] event       The event type to process.
 * @param[in] event_data  Optional pointer to event-related data (can be NULL).
 *
 * @note
 * - If the state machine is inactive (`SM_ACTIVE` not set) or has an invalid state index, the function exits silently.
 * - If a transition is defined but leads to an invalid target state index, execution exits silently.
 * - Tracing is conditionally compiled with `CONFIG_SM_TRACER` and sub-options.
 * - `SM_TREN` flag distinguishes between permitted and forbidden transitions during tracing.
 *
 * @warning
 * - Make sure `machine->states` and all associated transitions are correctly initialized.
 * - Improper `s2` indices in transitions can silently break execution without errors.
 *
 * @return None.
 */

static void sm_machine(sm_machine_t* machine, sm_event_type_t event, void* event_data)
{
    uint8_t i;                  // loop variable
    const sm_state_t* state;      // pointer to current state
    const sm_transition_t* transition;    // pointer to transition array of current state
    uint8_t transnum;                   // number of transitions in transition[]
    static bool s1neqs2;        // true, if start and target states are different
    static bool found;          // found a transition, either disabled or enabled

    if ((event != evNullEvent) &&
        (machine != NULL) &&
        ((machine->flags & SM_ACTIVE) != 0u) &&
        (machine->s1 < machine->sizes)
       ) {
        machine->event = event;
        machine->event_data = event_data;
// copy to local variables for faster execution
        state = &(machine->states[machine->s1]); // current state
        if (state != NULL) {
            transition = state->transitions;    // transition array of current state (or NULL)
            transnum = state->size;             // number of transitions in transition[]
        } else {
            transition = NULL;                  // found a NULL pointer in sm_state_t[machine->s1]
            transnum = 0u;
        }

// search for permitted transition that is triggered by event
        found = false;
        if (transition != NULL) {
            for (i = 0u; i < transnum; i++, transition++) {
                if (transition->event == event) {
                    found = true;   // used by lost event tracer; otherwise ignored
// check target state for validity -- if invalid s2 is found, break SM loop and silently exit
                    if (transition->s2 >= machine->sizes) {
                        break;
                    }
// check guard:
// transition is executed if (1) there is no guard or (2) the guard permits it (taking in respect polarity)
                    if ((transition->guard == NULL) || ((transition->guard(machine) ^ transition->gpol) == true)) {
                        machine->flags |= SM_TREN;
#if defined(CONFIG_SM_TRACER)
                        if (sm_is_trace_enabled(machine)) {
                            if (machine->trc != NULL) {
                                machine->trc(machine,false);
                            }
                        }
#endif  // defined(CONFIG_SM_TRACER)
// check if target state is different from start state
                        s1neqs2 = true;
                        if (transition->s2 == machine->s1) {
                            s1neqs2 = false;
                        }
// call of exit_action of s1, if there is a state change and if exit_action is defined for s1
                        if (s1neqs2 == true) {
                            if (state->exit_action != NULL) {
                                state->exit_action(machine);
                            }
                        }
// action on transition
                        if (transition->action != NULL) {
                            transition->action(machine);
                        }
#if defined(CONFIG_SM_TRACER)
                        if (sm_is_trace_enabled(machine)) {
                            if (machine->trm != NULL) {
                                machine->trm(machine,transition);
                            }
                        }
#endif  // defined(CONFIG_SM_TRACER)
// transition to s2
                        machine->s1 = transition->s2;
// call of entry_action, if there is a state change and if entry_action is defined for s2
                        if (s1neqs2 == true) {
                            state = &(machine->states[machine->s1]);
                            if (state->entry_action != NULL) {
                                 state->entry_action(machine);
                            }
                        }
#if defined(CONFIG_SM_TRACER)
                        if (sm_is_trace_enabled(machine)) {
                            if (machine->trc != NULL) {
                                machine->trc(machine,true);
                            }
                        }
#endif  // defined(CONFIG_SM_TRACER)
// transition is executed, so exit from loop
                        machine->flags &= ~SM_TREN;
                        break;
                    } else {
// forbidden transition
                        machine->flags &= ~SM_TREN;
#if defined(CONFIG_SM_TRACER)
                        if (sm_is_trace_enabled(machine)) {
                            if (machine->trc != NULL) {
                                machine->trc(machine,true);
                            }
                            if (machine->trm != NULL) {
                                machine->trm(machine,transition);
                            }
                        }
#endif  // defined(CONFIG_SM_TRACER)
                    }
                }
            }
        }
#if defined(CONFIG_SM_TRACER)
#if defined(CONFIG_SM_TRACER_LOSTEVENT)
        if ((found == false) && (machine->flags & SM_TRACE_LE)) {
            if (sm_is_trace_enabled(machine)) {
                if (machine->trle != NULL) {
                    machine->trle(machine);
                }
            }
        }
#endif  // defined(CONFIG_SM_TRACER_LOSTEVENT)
#endif  // defined(CONFIG_SM_TRACER)
    }
}

void sm_initialize(sm_machine_t* machine, sm_state_idx s1, uint8_t id, const sm_state_t* states, uint32_t sizes, void* ctx)
{
    if (machine != NULL) {
        machine->s1 = s1;
        machine->id = id;
        machine->states = states;
        machine->sizes = sizes;

        sm_set_context(machine,ctx);
        machine->flags = 0u;

#if defined(CONFIG_SM_TRACER)
        machine->trm = NULL;
        machine->trc = NULL;
        machine->trle = NULL;
#endif  // defined(CONFIG_SM_TRACER)
    }
}

uint8_t sm_get_id(sm_machine_t* machine)
{
    return (machine != NULL) ? machine->id : SM_INVALID;
}

uint8_t sm_get_current_state(sm_machine_t* machine)
{
    return (machine != NULL) ? machine->s1 : SM_INVALID;
}

// uint8_t SM_SetCurrentState(sm_machine_t* machine, sm_state_idx s1)
// Parameters:
//   sm_machine_t* machine - pointer to state machine
//   sm_state_idx s1 - state
// Return:
//   true: State Machine is put in s1 state
//   false: State Machine is not touched, because s1 is invalid

uint8_t sm_set_current_state(sm_machine_t* machine, sm_state_idx s1)
{
    if (machine != NULL) {
        if (s1 < machine->sizes) {
            machine->s1 = s1;
            return true;
        }
    }
    return false;
}

uint8_t sm_get_state_count(sm_machine_t* machine)
{
    return (machine != NULL) ? machine->sizes : SM_INVALID;
}

void sm_set_context(sm_machine_t* machine, void* ctx)
{
    if (machine != NULL) {
        machine->ctx = ctx;
    }
}

void* sm_get_context(sm_machine_t* machine)
{
    return (machine != NULL) ? machine->ctx : NULL;
}

void sm_start(sm_machine_t* machine, sm_state_idx s1)
{
    if (machine != NULL) {
        if (sm_is_activated(machine) == false) {
            if (sm_set_current_state(machine,s1) == true) {
                sm_activate(machine);
            }
        }
    }
}

void sm_start_with_event(sm_machine_t* machine, sm_state_idx s1, sm_event_type_t event)
{
    if (machine != NULL) {
        if (sm_is_activated(machine) == false) {
            if (event < sm_EVENTS_NUMBER) {
                if (sm_set_current_state(machine,s1) == true) {
                    sm_activate(machine);
                    sm_post_event(event);
                }
            }
        }
    }
}

void sm_activate(sm_machine_t* machine)
{
    if (machine != NULL) {
        machine->flags |= SM_ACTIVE;
    }
}

void sm_deactivate(sm_machine_t* machine)
{
    if (machine != NULL) {
        machine->flags &= ~SM_ACTIVE;
    }
}

bool sm_is_activated(sm_machine_t* machine)
{
    if (machine != NULL) {
        return ((machine->flags & SM_ACTIVE) != 0u) ? true : false;
    }
    return false;
}

#if defined(CONFIG_SM_TRACER)

void sm_trace_on(sm_machine_t* machine)
{
    if (machine != NULL) {
        machine->flags |= SM_TRACE;
    }
}

void sm_trace_off(sm_machine_t* machine)
{
    if (machine != NULL) {
        machine->flags &= ~SM_TRACE;
    }
}

void sm_trace_lost_event_on(sm_machine_t* machine)
{
    if (machine != NULL) {
        machine->flags |= SM_TRACE_LE;
    }
}

void sm_trace_lost_event_off(sm_machine_t* machine)
{
    if (machine != NULL) {
        machine->flags &= ~SM_TRACE_LE;
    }
}

void sm_set_tracers(sm_machine_t* machine, sm_transition_tracer_t trm, sm_context_tracer_t trc, sm_lostevent_tracer_t trle)
{
    if (machine != NULL) {
        machine->trm = trm;
        machine->trc = trc;
        machine->trle = trle;
    }
}

bool sm_is_trace_enabled(sm_machine_t* machine)
{
    if (machine != NULL) {
        return ((machine->flags & SM_TRACE) != 0) ? true : false;
    }
    return false;
}

#endif  // defined(CONFIG_SM_TRACER)

// end of sm.c
