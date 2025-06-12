// $Id$

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

static uint32_t registred_machines = 0u;
static SM_MACHINE* sm_list[SM_MAX_STATE_MACHINES] = { 0 };

// esp_err_t SM_register_state_machine(SM_MACHINE* machine)
// Input:
//  machine - a pointer to a state machine
// Output:
//  ESP_OK - the mahine is registred successfully
//  ESP_ERR_INVALID_ARG - null pointer was given as an argument
//  ESP_ERR_NO_MEM - no room for more state machines to be registred
// Desription: This function registers state machines to be run. It can register maximum SM_MAX_STATE_MACHINES.
// If needed, SM_MAX_STATE_MACHINES can be adjusted in app configuration. There is no deregistration provided.
esp_err_t SM_register_state_machine(SM_MACHINE* machine)
{
    if (machine == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (registred_machines >= SM_MAX_STATE_MACHINES) {
        return ESP_ERR_NO_MEM;
    }
    sm_list[registred_machines++] = machine;
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
        for (uint32_t i = 0; i < registred_machines; i++) {
            SM_Machine(sm_list[i],event_id, event_data);
        }
    }
}

// esp_err_t sm_post_event_with_data(EVENT_TYPE event, void* event_data, size_t data_size)
// Input: event: event to be posted to the sm event loop
//        event_data: pointer to data to be passed to the event handler
//        data_size: size of the data to be passed
// Output: error code from esp_event_post_to()
// Description: This function posts an event to sm_loop_handle. This event then triggers a call to sm_event_handler()/.
// The event handler then passes the event to all registered state machines.

esp_err_t sm_post_event_with_data(EVENT_TYPE event, void* event_data, size_t data_size)
{
    if (NULL != sm_loop_handle) {
        return esp_event_post_to(sm_loop_handle,SM_EVENTS,event,event_data,data_size,portMAX_DELAY);
    }
    else {
        return ESP_ERR_INVALID_ARG;
    }
}

// void sm_post_event(EVENT_TYPE event)
// Input: event to be posten to the sm event loop
// Output: error code from esp_event_post_to()
// Description: This function posts an event to sm_loop_handle. This event then triggers a call to sm_event_handler()

esp_err_t sm_post_event(EVENT_TYPE event)
{
    return sm_post_event_with_data(event,NULL,0u);
}

// void SM_machine(SM_MACHINE* machine)
// Parameters:
//   SM_MACHINE* machine - pointer to state machine
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
// 1. Bit SM_TREN in machine->flags is set up, because permitted transition is found
// 2. SM_TraceContext(sm,false) -- information before exiting s1
// 3. s1.exit_action            -- exit action of s1 if s1 != s2
// 4. tr.action                 -- transition action
// 5. SM_TraceMachine(sm,tr)    -- trace transition
// 6. s2.entry_action           -- entry action of s2 if s1 != s2
// 7. SM_TraceContext(sm,true)  -- information after entering s2
// 8. Bit SM_TREN in machine->flags is cleared
// 9. exit
//
// Forbidden transition:
// 1. SM_TraceContext(sm,true)  -- information for s1
// 2. SM_TraceMachine(sm,tr)    -- information for forbidden transition

// SM_TraceContext may distinguish permitted from not permitted transition by looking
// flag SM_TREN. If SM_TREN is 1 (true), transition is permitted.

void SM_Machine(SM_MACHINE* machine, EVENT_TYPE ev, void* event_data)
{
    uint8_t i;                  // loop variable
    const SM_STATE* st;         // pointer to current state
    const SM_TRANSITION* tr;    // pointer to transition array of current state
    uint8_t sz;                 // number of transitions in tr[]
    static bool s1neqs2;        // true, if start and target states are different
    static bool found;          // found a transition, either disabled or enabled

    if ((ev != evNullEvent) &&
        (machine != NULL) &&
        ((machine->flags & SM_ACTIVE) != 0u) &&
        (machine->s1 < machine->sizes)
       ) {
        machine->ev = ev;
        machine->event_data = event_data;
// copy to local variables for faster execution
        st = &(machine->states[machine->s1]); // current state
        if (st != NULL) {
            tr = st->tr;            // transition array of current state (or NULL)
            sz = st->size;          // number of transitions in tr[]
        } else {
            tr = NULL;              // found a NULL pointer in SM_STATE[machine->s1]
            sz = 0u;
        }

// search for permitted transition that is triggered by ev
        found = false;
        if (tr != NULL) {
            for (i = 0u; i < sz; i++, tr++) {
                if (tr->event == ev) {
                    found = true;   // used by lost event tracer; otherwise ignored
// check target state for validity -- if invalid s2 is found, break SM loop and silently exit
                    if (tr->s2 >= machine->sizes) {
                        break;
                    }
// check guard:
// transition is executed if (1) there is no guard or (2) the guard permits it (taking in respect polarity)
                    if ((tr->guard == NULL) || ((tr->guard(machine) ^ tr->gpol) == true)) {
                        machine->flags |= SM_TREN;
#if defined(CONFIG_SM_TRACER)
                        if (SM_IsTraceEnabled(machine) == true) {
                            if (machine->trc != NULL) {
                                machine->trc(machine,false);
                            }
                        }
#endif  // defined(CONFIG_SM_TRACER)
// check if target state is different from start state
                        s1neqs2 = true;
                        if (tr->s2 == machine->s1) {
                            s1neqs2 = false;
                        }
// call of exit_action of s1, if there is a state change and if exit_action is defined for s1
                        if (s1neqs2 == true) {
                            if (st->exit_action != NULL) {
                                st->exit_action(machine);
                            }
                        }
// action on transition
                        if (tr->a != NULL) {
                            tr->a(machine);
                        }
#if defined(CONFIG_SM_TRACER)
                        if (SM_IsTraceEnabled(machine) == true) {
                            if (machine->trm != NULL) {
                                machine->trm(machine,tr);
                            }
                        }
#endif  // defined(CONFIG_SM_TRACER)
// transition to s2
                        machine->s1 = tr->s2;
// call of entry_action, if there is a state change and if entry_action is defined for s2
                        if (s1neqs2 == true) {
                            st = &(machine->states[machine->s1]);
                            if (st->entry_action != NULL) {
                                 st->entry_action(machine);
                            }
                        }
#if defined(CONFIG_SM_TRACER)
                        if (SM_IsTraceEnabled(machine) == true) {
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
                        if (SM_IsTraceEnabled(machine) == true) {
                            if (machine->trc != NULL) {
                                machine->trc(machine,true);
                            }
                            if (machine->trm != NULL) {
                                machine->trm(machine,tr);
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
            if (SM_IsTraceEnabled(machine) == true) {
                if (machine->trle != NULL) {
                    machine->trle(machine);
                }
            }
        }
#endif  // defined(CONFIG_SM_TRACER_LOSTEVENT)
#endif  // defined(CONFIG_SM_TRACER)
    }
}

// static void SM_ClearFlags(SM_MACHINE* machine)
// Parameters:
//  SM_MACHINE* machine - pointer to state machine
// Return: no
// Description: Reset all flags to zero (false).

void SM_ClearFlags(SM_MACHINE* machine);

void SM_ClearFlags(SM_MACHINE* machine)
{
    if (machine != NULL) {
        machine->flags = 0u;
    }
}

// void SM_Initialize(SM_MACHINE* machine, STATE_TYPE s1, uint8_t id, const SM_STATE* states, uint32_t sizes, void* ctx)
// Parameters:
//   SM_MACHINE* machine - pointer to state machine
//   STATE_TYPE s1 - start (initial) state
//   uint8_t id - identifier of state machine
//   const SM_STATE* states - pointer to states array
//   uint32_t sizes - size of states array
//   void* ctx - pointer to user defined context structure
// Return: no
// Description: SM_Initialize initializes *machine fields with parameters.

void SM_Initialize(SM_MACHINE* machine, STATE_TYPE s1, uint8_t id, const SM_STATE* states, uint32_t sizes, void* ctx)
{
    if (machine != NULL) {
        machine->s1 = s1;
        machine->id = id;
        machine->states = states;
        machine->sizes = sizes;

        SM_SetContext(machine,ctx);
        SM_ClearFlags(machine);

#if defined(SM_TRCONFIG_SM_TRACERACER)
        machine->trm = NULL;
        machine->trc = NULL;
        machine->trle = NULL;
#endif  // defined(CONFIG_SM_TRACER)
    }
}

// uint8_t SM_GetID(SM_MACHINE* machine)
// Parameters:
//   SM_MACHINE* machine - pointer to state machine
// Return:
//   Identifier of state machine
// Description: SM_GetID returns the identifier of state machine *machine.

uint8_t SM_GetID(SM_MACHINE* machine)
{
    return (machine != NULL) ? machine->id : SM_INVALID;
}

// uint8_t SM_GetCurrentState(SM_MACHINE* machine)
// Parameters:
//   SM_MACHINE* machine - pointer to state machine
// Return:
//   current state of *machine
// Description: SM_GetCurrentState returns current state of the machine.

uint8_t SM_GetCurrentState(SM_MACHINE* machine)
{
    return (machine != NULL) ? machine->s1 : SM_INVALID;
}

// uint8_t SM_SetCurrentState(SM_MACHINE* machine, STATE_TYPE s1)
// Parameters:
//   SM_MACHINE* machine - pointer to state machine
//   STATE_TYPE s1 - state
// Return:
//   true: State Machine is put in s1 state
//   false: State Machine is not touched, because s1 is invalid

uint8_t SM_SetCurrentState(SM_MACHINE* machine, STATE_TYPE s1)
{
    if (machine != NULL) {
        if (s1 < machine->sizes) {
            machine->s1 = s1;
            return true;
        }
    }
    return false;
}

// uint8_t SM_GetStateCount(SM_MACHINE* machine)
// Parameters:
//   SM_MACHINE* machine - pointer to state machine
// Return:
//   number of states in *machine
// Description: SM_GetStateCount returns the number of states of *machine.

uint8_t SM_GetStateCount(SM_MACHINE* machine)
{
    return (machine != NULL) ? machine->sizes : SM_INVALID;
}

// void SM_SetContext(SM_MACHINE* machine, void* ctx)
// Parameters:
//   SM_MACHINE* machine - pointer to state machine
// Return: no
// Description: Sets context of the state machine pointed to by *machine.

void SM_SetContext(SM_MACHINE* machine, void* ctx)
{
    if (machine != NULL) {
        machine->ctx = ctx;
    }
}

// void* SM_GetContext(SM_MACHINE* machine)
// Parameters:
//   SM_MACHINE* machine - pointer to state machine
// Return: pointer to context structure
// Description: Return pointer to a structure that contains context information of *machine state machine.

void* SM_GetContext(SM_MACHINE* machine)
{
    return (machine != NULL) ? machine->ctx : NULL;
}

// void SM_Start(SM_MACHINE* machine, STATE_TYPE s1)
// Parameters:
//   SM_MACHINE* machine - pointer to state machine
//   STATE_TYPE s1 - start state
// Return: no
// Description: SM_Start puts the state machine pointed to by *machine in state s1 and activates it.
// If s1 is not valid machine function does not have any effect. It the state machine has been already
// activated, machine function does not do anything.
// After call of SM_StartWithEvent the caller should check if *machine is activated.

void SM_Start(SM_MACHINE* machine, STATE_TYPE s1)
{
    if (machine != NULL) {
        if (SM_IsActivated(machine) == false) {
            if (SM_SetCurrentState(machine,s1) == true) {
                SM_Activate(machine);
            }
        }
    }
}

// void SM_StartWithEvent(SM_MACHINE* machine, STATE_TYPE s1, EVENT_TYPE ev)
// Parameters:
//   SM_MACHINE* machine - pointer to state machine
//   STATE_TYPE s1 - start state
//   EVENT_TYPE ev - event that triggers *machine in s1
// Return: no
// Description: SM_StartWithEvent puts the state machine pointed to by *machine in state s1, activates it
// and then injects event ev. If s1 is not valid machine function does not have any effect. It the state machine
// has been already activated, machine function does not do anything.
// After call of SM_StartWithEvent the caller should check if *machine is activated.

void SM_StartWithEvent(SM_MACHINE* machine, STATE_TYPE s1, EVENT_TYPE ev)
{
    if (machine != NULL) {
        if (SM_IsActivated(machine) == false) {
            if (ev < evEVENTSNUMBER) {
                if (SM_SetCurrentState(machine,s1) == true) {
                    SM_Activate(machine);
                    sm_post_event(ev);
                }
            }
        }
    }
}

// void SM_Activate(SM_MACHINE* machine)
// Parameters:
//   SM_MACHINE* machine - pointer to state machine
// Return: no
// Description: SM_Activate activates *machine.

void SM_Activate(SM_MACHINE* machine)
{
    if (machine != NULL) {
        machine->flags |= SM_ACTIVE;
    }
}

// void SM_Deactivate(SM_MACHINE* machine)
// Parameters:
//   SM_MACHINE* machine - pointer to state machine
// Return: no
// Description: SM_Deactivate deactivates *machine. *machine will not act on any events.

void SM_Deactivate(SM_MACHINE* machine)
{
    if (machine != NULL) {
        machine->flags &= ~SM_ACTIVE;
    }
}

// uint8_t SM_IsActivated(SM_MACHINE* machine)
// Parameters:
//   SM_MACHINE* machine - pointer to state machine
// Return: true: *machine is active; false: *machine is not active
// Description: Determine if *machine is active.

uint8_t SM_IsActivated(SM_MACHINE* machine)
{
    if (machine != NULL) {
        return ((machine->flags & SM_ACTIVE) != 0u) ? true : false;
    }
    return false;
}

#if defined(CONFIG_SM_TRACER)

// void SM_TraceOn(SM_MACHINE* machine)
// Parameters:
//   SM_MACHINE* machine - pointer to state machine
// Return: no
// Description: SM_TraceOn enables tracing of *machine state machine

void SM_TraceOn(SM_MACHINE* machine)
{
    if (machine != NULL) {
        machine->flags |= SM_TRACE;
    }
}

// void SM_TraceOff(SM_MACHINE* machine)
// Parameters:
//   SM_MACHINE* machine - pointer to state machine
// Return: no
// Description: SM_TraceOff disables tracing of *machine state machine

void SM_TraceOff(SM_MACHINE* machine)
{
    if (machine != NULL) {
        machine->flags &= ~SM_TRACE;
    }
}

// void SM_TraceLostEventOn(SM_MACHINE* machine)
// Parameters:
//   SM_MACHINE* machine - pointer to state machine
// Return: no
// Description: SM_TraceLostEventOn enables tracing of lost events of *machine state machine

void SM_TraceLostEventOn(SM_MACHINE* machine)
{
    if (machine != NULL) {
        machine->flags |= SM_TRACE_LE;
    }
}

// void SM_TraceLostEventOff(SM_MACHINE* machine)
// Parameters:
//   SM_MACHINE* machine - pointer to state machine
// Return: no
// Description: SM_TraceLostEventOff disables tracing of lost events of *machine state machine
// Note: This function does not disable tracing of *machine state machine.

void SM_TraceLostEventOff(SM_MACHINE* machine)
{
    if (machine != NULL) {
        machine->flags &= ~SM_TRACE_LE;
    }
}

// void SM_SetTracers(SM_MACHINE* machine, SM_TRANSITION_TRACER trm, SM_CONTEXT_TRACER trc, SM_LOSTEVENT_TRACER trle)
// Parameters:
//  SM_MACHINE* machine - pointer to state machine
//  SM_TRANSITION_TRACER trm - pointer to a transition tracer
//  SM_CONTEXT_TRACER trc - pointer to a context tracer
//  SM_LOSTEVENT_TRACER trle - pointer to a lost events tracer
void SM_SetTracers(SM_MACHINE* machine, SM_TRANSITION_TRACER trm, SM_CONTEXT_TRACER trc, SM_LOSTEVENT_TRACER trle)
{
    if (machine != NULL) {
        machine->trm = trm;
        machine->trc = trc;
        machine->trle = trle;
    }
}

// uint8_t SM_IsTraceEnabled(SM_MACHINE* machine)
// Parameters:
//   SM_MACHINE* machine - pointer to state machine
// Return: true: trace is enabled; false: trace is disabled
// Description: Determine if tracing of *machine is enabled.

uint8_t SM_IsTraceEnabled(SM_MACHINE* machine)
{
    if (machine != NULL) {
        return ((machine->flags & SM_TRACE) != 0) ? true : false;
    }
    return false;
}

#endif  // defined(CONFIG_SM_TRACER)

// end of sm.c
