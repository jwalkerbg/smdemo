/*
 * State machine definitions and declarations
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

/*
 * Event loop definitions and declarations
 */

#define SM_MAX_STATE_MACHINES   (CONFIG_SM_MAX_STATE_MACHINES)

#define SM_EVENT_LOOP_QUEUE_SIZE    (CONFIG_SM_EVENT_LOOP_QUEUE_SIZE)
#define SM_EVENT_TASK               ("sm_evt_task")
#define SM_EVENT_TASK_PRIORITY      (1)
#define SM_EVENT_TASK_STACK_SIZE    (CONFIG_SM_EVENT_TASK_STACK_SIZE)

#if defined(CONFIG_SM_EVENT_TYPE_DEFINED_IN_APPLICATION)
#include "events.h"
#else

/* TODO: Add real events here. Do not remove evNullEvent and sm_EVENTS_NUMBER */
typedef enum {
    evNullEvent,

    evEVENT1,
    evEVENT2,

    sm_EVENTS_NUMBER
} sm_event_type_t;

#endif  /* !defined(CONFIG_SM_EVENT_TYPE_DEFINED_IN_APPLICATION) */

/**
 * @brief Creates an event loop dedicated to the state machine component.
 *
 * This function sets up a dedicated event loop for handling state machine (SM) events.
 * It initializes the loop and registers an event handler to manage incoming SM-related events,
 * such as those originating from BLE or other sources.
 *
 * @note The event loop is created using `esp_event_loop_create()`, and a handler is registered with it
 * to listen to all events under the `SM_EVENTS` base. This setup must be completed before posting
 * events to the SM event loop.
 *
 * @return
 *  - ESP_OK: The event loop and handler were successfully created and registered.
 *  - Any other `esp_err_t` code returned from `esp_event_loop_create()` or `esp_event_handler_instance_register_with()`
 *    in case of failure.
 */
esp_err_t sm_create_event_loop(void);

/**
 * @brief Posts an event to the state machine event loop.
 *
 * This function posts an event to the state machine event loop, which is then processed by the
 * registered state machines. The event can be any valid `sm_event_type_t` defined in the application.
 *
 * @param[in] event The event to post to the state machine event loop.
 *
 * @return
 *  - ESP_OK: The event was posted successfully.
 *  - ESP_ERR_INVALID_ARG: The input event is invalid or the event loop is not initialized.
 */
esp_err_t sm_post_event(sm_event_type_t event);

/**
 * @brief Posts an event with associated data to the state machine event loop.
 *
 * This function posts an event along with additional data to the state machine event loop.
 * The data can be used by the state machines to process the event with context-specific information.
 *
 * @param[in] event The event to post to the state machine event loop.
 * @param[in] event_data Pointer to the data associated with the event (can be NULL).
 * @param[in] data_size Size of the data pointed to by `event_data`.
 *
 * @return
 *  - ESP_OK: The event and data were posted successfully.
 *  - ESP_ERR_INVALID_ARG: The input parameters are invalid or the event loop is not initialized.
 */
esp_err_t sm_post_event_with_data(sm_event_type_t event, void* event_data, size_t data_size);

/* forward definition of types */
typedef uint8_t sm_state_idx_t;
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

/* transition */
#define SM_GPOL_POSITIVE    (0u)
#define SM_GPOL_NEGATIVE    (1u)
struct sm_transition {
    sm_event_type_t event;  // trigger event
    sm_state_idx_t s2;      // destination state (index in an array)
    sm_action_t action;     // action (can be NULL: null action)
    uint8_t actidx;         // action index
    sm_guard_t guard;       // guard: true: transition is permitted, false: transition is forbidden;
                            // (can be NULL, then transition is permitted
    uint8_t gpol;           // guard polarity: SM_GPOL_POSITIVE or SM_GPOL_NEGATIVE
};

/* state */
struct sm_state {
    const sm_transition_t* transitions; // array of exit transitions (can be NULL: no exit transition from the state)
    uint32_t size;                      // number of transitions in tr[]
    sm_action_t entry_action;           // action on entering state, may be NULL
    sm_action_t exit_action;            // action on exiting state, may be NULL
};

/* state machine */

/* masks for .flags */
#define SM_ACTIVE   (0b00000001u)   // the state machine is active (accepts events)
#define SM_TREN     (0b00000010u)   // true: found transition is permitted by transition->guard
#define SM_TRACE    (0b10000000u)   // tracing, if compiled, is enabled
#define SM_TRACE_LE (0b01000000u)   // tracing lost events, if compiled, is enabled

#define SM_INVALID (255u)           // no valid response is possible

struct sm_machine {
    sm_state_idx_t s1;          // current state of state machine (index)
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

/**
 * @brief Registers a state machine to be managed by the state machine component.
 *
 * This function registers a state machine so that it can be executed by the state machine runner.
 * The maximum number of state machines that can be registered is defined by `SM_MAX_STATE_MACHINES`,
 * which can be configured via the application configuration.
 *
 * @note There is no deregistration function available. Once registered, a state machine remains
 * active until the system is restarted or reset.
 *
 * @param[in] machine Pointer to the state machine to register.
 *
 * @return
 *  - ESP_OK: The state machine was registered successfully.
 *  - ESP_ERR_INVALID_ARG: The input pointer was NULL.
 *  - ESP_ERR_NO_MEM: The maximum number of state machines has been reached.
 */
esp_err_t sm_register_state_machine(sm_machine_t* machine);

/**
 * @brief Initializes a state machine instance with the given configuration.
 *
 * This function sets up the internal fields of a `sm_machine_t` object, including its
 * initial state, identifier, state array, size, and context. It also clears internal flags
 * and resets optional tracing function pointers if tracing is enabled.
 *
 * @param[out] machine Pointer to the state machine instance to initialize.
 * @param[in]  s1      Index of the initial (start) state.
 * @param[in]  id      Identifier assigned to this state machine instance.
 * @param[in]  states  Pointer to an array of state definitions (`sm_state_t`).
 * @param[in]  sizes   Number of elements in the `states` array.
 * @param[in]  ctx     Pointer to user-defined context data for this machine.
 *
 * @note
 * - The function does nothing if `machine` is `NULL`.
 * - Tracing fields (`trm`, `trc`, `trle`) are reset to `NULL` if `CONFIG_SM_TRACER` is defined.
 *
 * @warning
 * - Ensure that the `states` array and its content remain valid for the lifetime of the machine.
 * - The context pointer is stored directly; no copying or validation is performed.
 *
 * @return None.
 */
void sm_initialize(sm_machine_t* machine, sm_state_idx_t s1, uint8_t id, const sm_state_t* states, uint32_t sizes, void* ctx);

/**
 * @brief Retrieves the identifier of a state machine instance.
 *
 * Returns the identifier assigned to the given state machine. If the pointer is `NULL`,
 * the function returns `SM_INVALID`.
 *
 * @param[in] machine Pointer to the state machine instance.
 *
 * @return
 * - The identifier of the state machine.
 * - `SM_INVALID` if `machine` is `NULL`.
 *
 * @note `SM_INVALID` should be defined appropriately (e.g., as `0xFF`) to indicate an invalid ID.
 */
uint8_t sm_get_id(sm_machine_t* machine);

/**
 * @brief Retrieves the current active state of the state machine.
 *
 * Returns the index of the currently active state (`s1`) of the specified state machine.
 * If the pointer is `NULL`, the function returns `SM_INVALID`.
 *
 * @param[in] machine Pointer to the state machine instance.
 *
 * @return
 * - Index of the current state.
 * - `SM_INVALID` if `machine` is `NULL`.
 *
 * @note `SM_INVALID` should be a defined constant representing an invalid state (e.g., `0xFF`).
 */
uint8_t sm_get_current_state(sm_machine_t* machine);

/**
 * @brief Sets the current active state of the state machine.
 *
 * Attempts to set the state machine's current state to the specified state index `s1`.
 * The state is only updated if `s1` is within the valid range of states.
 *
 * @param[in,out] machine Pointer to the state machine instance.
 * @param[in]     s1      The state index to set as the current state.
 *
 * @return
 * - `true` if the state was successfully updated.
 * - `false` if the state machine pointer is `NULL` or the state index `s1` is invalid.
 *
 * @note This function does not execute any entry/exit actions or transitions;
 * it simply changes the current state index.
 */
bool sm_set_current_state(sm_machine_t* machine, sm_state_idx_t s1);

/**
 * @brief Returns the number of states defined in the state machine.
 *
 * Retrieves the total count of states configured for the specified state machine.
 * If the pointer is `NULL`, returns `SM_INVALID`.
 *
 * @param[in] machine Pointer to the state machine instance.
 *
 * @return
 * - Number of states in the machine.
 * - `SM_INVALID` if `machine` is `NULL`.
 *
 * @note `SM_INVALID` should be defined to indicate an invalid or error value (e.g., `0xFF`).
 */
sm_state_idx_t sm_get_state_count(sm_machine_t* machine);

/**
 * @brief Sets the user-defined context for the state machine.
 *
 * Assigns a user-provided context pointer to the given state machine.
 * This context can be accessed by actions, guards, and other state-specific logic.
 *
 * @param[in,out] machine Pointer to the state machine instance.
 * @param[in]     ctx     Pointer to user-defined context data.
 *
 * @note The context is not validated or modified by the state machine framework.
 */
void sm_set_context(sm_machine_t* machine, void* ctx);

/**
 * @brief Retrieves the user-defined context from the state machine.
 *
 * Returns the pointer to the context structure previously set with `sm_set_context()`.
 * This context can hold custom user data associated with the state machine.
 *
 * @param[in] machine Pointer to the state machine instance.
 *
 * @return
 * - Pointer to the context structure.
 * - `NULL` if `machine` is `NULL` or no context is set.
 *
 * @see sm_set_context
 */
void* sm_get_context(sm_machine_t* machine);

/**
 * @brief Starts the state machine in a specified initial state.
 *
 * Sets the state machine to state `s1` and marks it as active. If the state machine
 * is already active or the specified state index is invalid, this function has no effect.
 *
 * @param[in,out] machine Pointer to the state machine instance.
 * @param[in]     s1      The state index to start the machine in.
 *
 * @note After calling this function, use `sm_is_activated()` to verify activation status.
 *
 * @see sm_is_activated
 * @see sm_set_current_state
 * @see sm_activate
 */
void sm_start(sm_machine_t* machine, sm_state_idx_t s1);

/**
 * @brief Starts the state machine in a specified initial state and posts an event.
 *
 * Sets the state machine to state `s1`, activates it, and then injects the specified event.
 * If the state machine is already active, or if the state or event is invalid, this function has no effect.
 *
 * @param[in,out] machine Pointer to the state machine instance.
 * @param[in]     s1      The state index to start the machine in.
 * @param[in]     event   The event to inject after activation.
 *
 * @note After calling this function, use `sm_is_activated()` to check activation status.
 *       Event must be less than `sm_EVENTS_NUMBER` to be valid.
 *
 * @see sm_start
 * @see sm_is_activated
 * @see sm_activate
 * @see sm_post_event
 */
void sm_start_with_event(sm_machine_t* machine, sm_state_idx_t s1, sm_event_type_t ev);

/**
 * @brief Activates the specified state machine.
 *
 * Sets the internal flag to mark the state machine as active.
 * Does nothing if the provided pointer is NULL.
 *
 * @param[in,out] machine Pointer to the state machine to be activated.
 *
 * @see sm_is_activated
 * @see sm_start
 * @see sm_start_with_event
 */
void sm_activate(sm_machine_t* machine);

/**
 * @brief Deactivates the specified state machine.
 *
 * Clears the active flag of the state machine, preventing it from responding to any further events.
 * Does nothing if the provided pointer is NULL.
 *
 * @param[in,out] machine Pointer to the state machine to deactivate.
 *
 * @see sm_activate
 * @see sm_is_activated
 */
void sm_deactivate(sm_machine_t* machine);

/**
 * @brief Checks whether the specified state machine is active.
 *
 * Evaluates the internal flag of the state machine to determine if it is currently active.
 * Returns false if the machine pointer is NULL.
 *
 * @param[in] machine Pointer to the state machine to check.
 *
 * @return true if the machine is active, false otherwise.
 *
 * @see sm_activate
 * @see sm_deactivate
 */
bool sm_is_activated(sm_machine_t* machine);

#if defined(CONFIG_SM_TRACER)

/* trace data */
struct sm_tracedata {
    uint32_t time;          // time when transition is executed
    uint8_t id;             // state machine identifier
    sm_state_idx_t s1;      // start state
    sm_state_idx_t s2;      // target state
    sm_event_type_t ev;     // trigger event
};
typedef struct sm_tracedata SM_TRACEDATA;

/**
 * @brief Enables tracing for the specified state machine.
 *
 * Sets the SM_TRACE flag in the state machine's flags field, enabling trace functionality.
 *
 * @param[in] machine Pointer to the state machine.
 *
 * @see sm_trace_off
 * @see sm_is_trace_enabled
 */
void sm_trace_on(sm_machine_t* machine);

/**
 * @brief Disables tracing for the specified state machine.
 *
 * Clears the SM_TRACE flag in the state machine's flags field, disabling trace functionality.
 *
 * @param[in] machine Pointer to the state machine.
 *
 * @see sm_trace_on
 * @see sm_is_trace_enabled
 */
void sm_trace_off(sm_machine_t* machine);

/**
 * @brief Enables tracing of lost events for the specified state machine.
 *
 * Sets the SM_TRACE_LE flag in the state machine's flags field, which allows
 * the tracing system to record when an event does not result in a valid or permitted transition.
 *
 * @param[in] machine Pointer to the state machine.
 *
 * @see sm_trace_lost_event_off
 * @see sm_is_trace_enabled
 */
void sm_trace_lost_event_on(sm_machine_t* machine);

/**
 * @brief Disables tracing of lost events for the specified state machine.
 *
 * Clears the SM_TRACE_LE flag in the state machine's flags field, stopping the tracing
 * of lost events (events that do not trigger any permitted transition).
 *
 * @note This function does not disable the overall tracing of the state machine.
 *
 * @param[in] machine Pointer to the state machine.
 *
 * @see sm_trace_lost_event_on
 * @see sm_trace_on
 * @see sm_trace_off
 */
void sm_trace_lost_event_off(sm_machine_t* machine);

/**
 * @brief Sets tracer callbacks for the specified state machine.
 *
 * Assigns the transition tracer, context tracer, and lost events tracer functions
 * to the given state machine. These tracers are used for monitoring and debugging
 * state machine behavior.
 *
 * @param[in,out] machine Pointer to the state machine.
 * @param[in] trm Pointer to the transition tracer function.
 * @param[in] trc Pointer to the context tracer function.
 * @param[in] trle Pointer to the lost events tracer function.
 *
 * @note All tracer pointers can be set to NULL to disable the corresponding tracing.
 */
void sm_set_tracers(sm_machine_t* machine, sm_transition_tracer_t trm, sm_context_tracer_t trc, sm_lostevent_tracer_t trle);

/**
 * @brief Checks if tracing is enabled for the given state machine.
 *
 * @param[in] machine Pointer to the state machine.
 * @return true if tracing is enabled; false otherwise.
 *
 * @note Returns false if the machine pointer is NULL.
 */
bool sm_is_trace_enabled(sm_machine_t* machine);

#endif  // defined(CONFIG_SM_TRACER)

#ifdef __cplusplus
}
#endif

// end of sm.h
