// $Id$

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"

#include "sm.h"

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

// esp_err_t sm_create_event_loop(void)
// Input: none
// Output: error code
// Description: This function creates event loop specifically for SM.

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

    // register hander for commands incoming from BLE
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

// esp_err_t sm_register_state_machine(sm_machine_t* machine)
// Input:
//  machine - a pointer to a state machine
// Output:
//  ESP_OK - the mahine is registered successfully
//  ESP_ERR_INVALID_ARG - null pointer was given as an argument
//  ESP_ERR_NO_MEM - no room for more state machines to be registered
// Desription: This function registers state machines to be run. It can register maximum SM_MAX_STATE_MACHINES.
// If needed, SM_MAX_STATE_MACHINES can be adjusted in app configuration. There is no deregistration provided.
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

// static void sm_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
// Input: event_id: the event that has to be passed to the state machines
//        The other arguments are not used.
// Output: none
// Description: This event handler passes the received event (event_id) to the state machines. See the example below in the fuction.

static void sm_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_id != evNullEvent) {
        // Send events to state machines.
        for (uint32_t i = 0; i < registered_machines; i++) {
            sm_machine(sm_list[i],event_id, event_data);
        }
    }
}

// esp_err_t sm_post_event_with_data(sm_event_type_t event, void* event_data, size_t data_size)
// Input: event: event to be posted to the sm event loop
//        event_data: pointer to data to be passed to the event handler
//        data_size: size of the data to be passed
// Output: error code from esp_event_post_to()
// Description: This function posts an event to sm_loop_handle. This event then triggers a call to sm_event_handler()/.
// The event handler then passes the event to all registered state machines.

esp_err_t sm_post_event_with_data(sm_event_type_t event, void* event_data, size_t data_size)
{
    if (NULL != sm_loop_handle) {
        return esp_event_post_to(sm_loop_handle,SM_EVENTS,event,event_data,data_size,portMAX_DELAY);
    }
    else {
        return ESP_ERR_INVALID_ARG;
    }
}

// void sm_post_event(sm_event_type_t event)
// Input: event to be posted to the sm event loop
// Output: error code from esp_event_post_to()
// Description: This function posts an event to sm_loop_handle. This event then triggers a call to sm_event_handler()

esp_err_t sm_post_event(sm_event_type_t event)
{
    return sm_post_event_with_data(event,NULL,0u);
}

// void SM_machine(sm_machine_t* machine)
// Parameters:
//   sm_machine_t* machine - pointer to state machine
// Return: no
// Description: SM_machine searches for a transition to be triggered by event event.
// If it finds transition, it checks if it is permitted by guard (if any). If the
// transition is found and permitted, it is executed. If there is not permitted transition
// that could be triggered by event SM_Machine does nothing.
// Note: If tracing is enabled, it is executed and for found but not permitted transitions.
//
// Order of execution:
//
// Permitted transition:
// 1. Bit SM_TREN in machine->flags is set up, because permitted transition is found
// 2. SM_TraceContext(sm,false) -- information before exiting s1
// 3. s1.exit_action            -- exit action of s1 if s1 != s2
// 4. transition.action                 -- transition action
// 5. SM_TraceMachine(sm,transition)    -- trace transition
// 6. s2.entry_action           -- entry action of s2 if s1 != s2
// 7. SM_TraceContext(sm,true)  -- information after entering s2
// 8. Bit SM_TREN in machine->flags is cleared
// 9. exit
//
// Forbidden transition:
// 1. SM_TraceContext(sm,true)  -- information for s1
// 2. SM_TraceMachine(sm,transition)    -- information for forbidden transition

// SM_TraceContext may distinguish permitted from not permitted transition by looking
// flag SM_TREN. If SM_TREN is 1 (true), transition is permitted.

void sm_machine(sm_machine_t* machine, sm_event_type_t event, void* event_data)
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

// static void SM_ClearFlags(sm_machine_t* machine)
// Parameters:
//  sm_machine_t* machine - pointer to state machine
// Return: no
// Description: Reset all flags to zero (false).

void SM_ClearFlags(sm_machine_t* machine);

void SM_ClearFlags(sm_machine_t* machine)
{
    if (machine != NULL) {
        machine->flags = 0u;
    }
}

// void SM_Initialize(sm_machine_t* machine, sm_state_idx s1, uint8_t id, const sm_state_t* states, uint32_t sizes, void* ctx)
// Parameters:
//   sm_machine_t* machine - pointer to state machine
//   sm_state_idx s1 - start (initial) state
//   uint8_t id - identifier of state machine
//   const sm_state_t* states - pointer to states array
//   uint32_t sizes - size of states array
//   void* ctx - pointer to user defined context structure
// Return: no
// Description: SM_Initialize initializes *machine fields with parameters.

void sm_initialize(sm_machine_t* machine, sm_state_idx s1, uint8_t id, const sm_state_t* states, uint32_t sizes, void* ctx)
{
    if (machine != NULL) {
        machine->s1 = s1;
        machine->id = id;
        machine->states = states;
        machine->sizes = sizes;

        sm_set_context(machine,ctx);
        SM_ClearFlags(machine);

#if defined(CONFIG_SM_TRACER)
        machine->trm = NULL;
        machine->trc = NULL;
        machine->trle = NULL;
#endif  // defined(CONFIG_SM_TRACER)
    }
}

// uint8_t SM_GetID(sm_machine_t* machine)
// Parameters:
//   sm_machine_t* machine - pointer to state machine
// Return:
//   Identifier of state machine
// Description: SM_GetID returns the identifier of state machine *machine.

uint8_t sm_get_id(sm_machine_t* machine)
{
    return (machine != NULL) ? machine->id : SM_INVALID;
}

// uint8_t SM_GetCurrentState(sm_machine_t* machine)
// Parameters:
//   sm_machine_t* machine - pointer to state machine
// Return:
//   current state of *machine
// Description: SM_GetCurrentState returns current state of the machine.

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

// uint8_t sm_get_state_count(sm_machine_t* machine)
// Parameters:
//   sm_machine_t* machine - pointer to state machine
// Return:
//   number of states in *machine
// Description: sm_get_state_count returns the number of states of *machine.

uint8_t sm_get_state_count(sm_machine_t* machine)
{
    return (machine != NULL) ? machine->sizes : SM_INVALID;
}

// void sm_set_context(sm_machine_t* machine, void* ctx)
// Parameters:
//   sm_machine_t* machine - pointer to state machine
// Return: no
// Description: Sets context of the state machine pointed to by *machine.

void sm_set_context(sm_machine_t* machine, void* ctx)
{
    if (machine != NULL) {
        machine->ctx = ctx;
    }
}

// void* sm_get_context(sm_machine_t* machine)
// Parameters:
//   sm_machine_t* machine - pointer to state machine
// Return: pointer to context structure
// Description: Return pointer to a structure that contains context information of *machine state machine.

void* sm_get_context(sm_machine_t* machine)
{
    return (machine != NULL) ? machine->ctx : NULL;
}

// void sm_start(sm_machine_t* machine, sm_state_idx s1)
// Parameters:
//   sm_machine_t* machine - pointer to state machine
//   sm_state_idx s1 - start state
// Return: no
// Description: sm_start puts the state machine pointed to by *machine in state s1 and activates it.
// If s1 is not valid machine function does not have any effect. It the state machine has been already
// activated, machine function does not do anything.
// After call of sm_start_with_event the caller should check if *machine is activated.

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

// void sm_start_with_event(sm_machine_t* machine, sm_state_idx s1, sm_event_type_t event)
// Parameters:
//   sm_machine_t* machine - pointer to state machine
//   sm_state_idx s1 - start state
//   sm_event_type_t event - event that triggers *machine in s1
// Return: no
// Description: sm_start_with_event puts the state machine pointed to by *machine in state s1, activates it
// and then injects event event. If s1 is not valid machine function does not have any effect. It the state machine
// has been already activated, machine function does not do anything.
// After call of sm_start_with_event the caller should check if *machine is activated.

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

// void sm_activate(sm_machine_t* machine)
// Parameters:
//   sm_machine_t* machine - pointer to state machine
// Return: no
// Description: sm_activate activates *machine.

void sm_activate(sm_machine_t* machine)
{
    if (machine != NULL) {
        machine->flags |= SM_ACTIVE;
    }
}

// void sm_deactivate(sm_machine_t* machine)
// Parameters:
//   sm_machine_t* machine - pointer to state machine
// Return: no
// Description: sm_deactivate deactivates *machine. *machine will not act on any events.

void sm_deactivate(sm_machine_t* machine)
{
    if (machine != NULL) {
        machine->flags &= ~SM_ACTIVE;
    }
}

// bool sm_is_activated(sm_machine_t* machine)
// Parameters:
//   sm_machine_t* machine - pointer to state machine
// Return: true: *machine is active; false: *machine is not active
// Description: Determine if *machine is active.

bool sm_is_activated(sm_machine_t* machine)
{
    if (machine != NULL) {
        return ((machine->flags & SM_ACTIVE) != 0u) ? true : false;
    }
    return false;
}

#if defined(CONFIG_SM_TRACER)

// void sm_trace_on(sm_machine_t* machine)
// Parameters:
//   sm_machine_t* machine - pointer to state machine
// Return: no
// Description: sm_trace_on enables tracing of *machine state machine

void sm_trace_on(sm_machine_t* machine)
{
    if (machine != NULL) {
        machine->flags |= SM_TRACE;
    }
}

// void sm_trace_off(sm_machine_t* machine)
// Parameters:
//   sm_machine_t* machine - pointer to state machine
// Return: no
// Description: sm_trace_off disables tracing of *machine state machine

void sm_trace_off(sm_machine_t* machine)
{
    if (machine != NULL) {
        machine->flags &= ~SM_TRACE;
    }
}

// void sm_trace_lost_event_on(sm_machine_t* machine)
// Parameters:
//   sm_machine_t* machine - pointer to state machine
// Return: no
// Description: sm_trace_lost_event_on enables tracing of lost events of *machine state machine

void sm_trace_lost_event_on(sm_machine_t* machine)
{
    if (machine != NULL) {
        machine->flags |= SM_TRACE_LE;
    }
}

// void sm_trace_lost_event_off(sm_machine_t* machine)
// Parameters:
//   sm_machine_t* machine - pointer to state machine
// Return: no
// Description: sm_trace_lost_event_off disables tracing of lost events of *machine state machine
// Note: This function does not disable tracing of *machine state machine.

void sm_trace_lost_event_off(sm_machine_t* machine)
{
    if (machine != NULL) {
        machine->flags &= ~SM_TRACE_LE;
    }
}

// void sm_set_tracers(sm_machine_t* machine, sm_transition_tracer_t trm, sm_context_tracer_t trc, sm_lostevent_tracer_t trle)
// Parameters:
//  sm_machine_t* machine - pointer to state machine
//  sm_transition_tracer_t trm - pointer to a transition tracer
//  sm_context_tracer_t trc - pointer to a context tracer
//  sm_lostevent_tracer_t trle - pointer to a lost events tracer
void sm_set_tracers(sm_machine_t* machine, sm_transition_tracer_t trm, sm_context_tracer_t trc, sm_lostevent_tracer_t trle)
{
    if (machine != NULL) {
        machine->trm = trm;
        machine->trc = trc;
        machine->trle = trle;
    }
}

// bool sm_is_trace_enabled(sm_machine_t* machine)
// Parameters:
//   sm_machine_t* machine - pointer to state machine
// Return: true: trace is enabled; false: trace is disabled
// Description: Determine if tracing of *machine is enabled.

bool sm_is_trace_enabled(sm_machine_t* machine)
{
    if (machine != NULL) {
        return ((machine->flags & SM_TRACE) != 0) ? true : false;
    }
    return false;
}

#endif  // defined(CONFIG_SM_TRACER)

// end of sm.c
