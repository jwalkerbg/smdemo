# State Machine

- [State Machine](#state-machine)
- [Formal definition.](#formal-definition)
  - [Definition](#definition)
  - [Guards](#guards)
- [Implementation. Data.](#implementation-data)
  - [Events](#events)
  - [States](#states)
  - [Transitions](#transitions)
  - [State machine object](#state-machine-object)
  - [Context](#context)
- [Trace](#trace)
  - [Trace state machine](#trace-state-machine)
  - [Trace context](#trace-context)
  - [Trace lost events](#trace-lost-events)
- [Implementation. Functions.](#implementation-functions)
  - [sm\_machine](#sm_machine)
  - [sm\_initialize](#sm_initialize)
  - [sm\_get\_id](#sm_get_id)
  - [sm\_get\_current\_state](#sm_get_current_state)
  - [sm\_set\_current\_state](#sm_set_current_state)
  - [sm\_get\_state\_count](#sm_get_state_count)
  - [sm\_set\_context](#sm_set_context)
  - [sm\_get\_context](#sm_get_context)
  - [sm\_start](#sm_start)
  - [sm\_start\_with\_event](#sm_start_with_event)
  - [sm\_activate](#sm_activate)
  - [sm\_deactivate](#sm_deactivate)
  - [sm\_is\_activated](#sm_is_activated)
  - [sm\_trace\_on](#sm_trace_on)
  - [sm\_trace\_off](#sm_trace_off)
  - [sm\_trace\_lost\_event\_on](#sm_trace_lost_event_on)
  - [sm\_trace\_lost\_event\_off](#sm_trace_lost_event_off)
  - [sm\_set\_tracers](#sm_set_tracers)
  - [sm\_is\_trace\_enabled](#sm_is_trace_enabled)

SM is an implementation of a event driven finite state machine written in C language.

The state machine is implemented as a single C function that performs a transition between states (if found suitable one and permitted). States and transitions are stored in tables, so the state machine function uses these tables to select transition for execution. Besides this function additional supplemental functions are provided to initialize and maintain the state machine.

State machine function does not loop. On each call, it receives an event, performs a transition and exits. This means that the applications that use SM must create a loop in which the state machine function will be called. This is conventient that it allows different main loops to be created suitable for different applicatios.

Nevertheless SM is written in C, it is created object oriented as much as posiible. Most of the functions receive as first parameter a pointer to a structure, that represents the state machine. If one needs, he would convert this code in C++ easily.

State machine helps separate the logic from actions. It makes clear separation between input, logic and output. This is essential for creating stable and extendable applications without lossing the full picture.

# Formal definition.

## Definition

State machine (Finite state machine - FSM) is an abstract machine that

- has finite number of states and it can be in exactly one of these states at any given time. One of these states is a start state.
- has finite number of input events that the machine reacts to
- has finite number of transtions between the states.
- has finite number of output actions that the machine executes when performing transitions
- each state may be assigned an entry action and an exit action
- each transition may be assigned a logical guard function

The work of FSM is to perform transitions step by step. The FSM, depending on the current state ```S1``` and the current input event ```Event```, chooses a transition and executes it to a new state ```S2```. In "the middle" of the transition it executes an output action. A transition is executed if it has no guard or the if the guard permits the transition. On permitted transition, following happens:

- if the current state has an exit action, it is executed if the new state differ from the current state.
- if the transition has defined an action it is executed
- If the new state has an entry action it is executed if the new state differ from the current state.

## Guards

Guards that are assigned to transitions are pairs of logical function and polarity. A guard permits a tranistion when its logical result xor'ed with the polarity gives result ```true```. One logical function may be part of two or more transitions with different polarities. This fact may be used to create some non-determinism in the state machine.

# Implementation. Data.

## Events

Events are reprsented as values of type ```sm_event_type_t```. It is defined as an ```enum```. Example:

```c
enum events_list {
    evNullEvent,
// TO DO: insert application events here
	  evButtonPressed, evButtonReleased,

    sm_EVENTS_NUMBER
};
typedef enum events_list sm_event_type_t;
```

Events are just values in ```sm_event_type_t```. The first event, ```evNullEvent``` is a special no-event value (when there is no new event in the input). It has value of zero. All other event which are application specific and must be inserted after ```evNullEvent```. The last element is ```sm_EVENTS_NUMBER``` which represents the number of items in the application that the FSM can accept.

The names of application specific events must be prefixed with ```ev``` prefix to avoid conflicts with values of other enums that may be defined in the application.

Events in the type cannot have gaps. This means that ```evSomeEvent = 10``` is not allowed. Several events cannot produce equal integer values.

The names of the events shall represent its semantic. Example: ```evDoorOpened```, ```evDoorClosed``` which may originate from some door sensor. ```evMeasurementsReady``` may originate from an low level algorithm that measures analog levels of some analog (ADC) inputs. Events usually represent changes of states of some sub-systems in the application. Taking above name ```evDoorOpened```, it means that the door has been closed (initial state) and it is opened now (the new state). The event tells that it was opened somehow - by a person, by the wind and so on.

## States

States in FSM have individual codes, that are presented by ```enums```. It is convenient that each state machine in the applicaion to have its own definition of states. And example:

The state machine is called ```P1`` (the process P1). Its states are:

```c
enum sP1_states {
    sP1_START, sP1_RESOLVE, sP1_F1, sP1_F2, sP1_F3, sP1_F4,

    sP1_StatesNumber
};
```

Here, the naming is as follows:

- ```s``` - indicates state
- ```P1``` - is the name of the state machine - par example it implements the behaviour of an application process / task / thread, named ```P1```.
- ```START```, ```RESOLVE``` etc are the names of the states. These are selected / created by the application programmer. Values in these enums serve as index in the array of states, associated with the state machine (see below).

Note that it is good idea the last item in an enum to be a special key which repersents the total number of states.

Each state may be assigned an entry and an exit action. These actions are functions with signature ```sm_action_t```, that are called on executing transitions between the states. What they will do is up to the application programmer. The states have set (array) of transitions which will be executed on arriving correspondent events. A state may have not transitions. This means that FSM will not react to any event. It looks like the state machine has dead.

Each state have a structure of data. Here it is:

```c
struct sm_state {
    const sm_transition_t* tr;    // array of exit transitions (can be NULL: no exit transition from the state)
    int size;                     // number of transitions in tr[]
    sm_action_t entry_action;     // action on entering state, may be NULL
    sm_action_t exit_action;      // action on exiting state, may be NULL
};
typedef struct sm_state sm_state_t;
```
The comments explain what the members are. Transitions are explained below. Here are some words about actions. The type ```sm_action_t``` is defined as follows:

```
typedef void (*sm_action_t)(sm_machine_t* machine);
```

This is a pointer to a C function returning nothing and accepting a pointer to the state machine object. ```entry_action``` is executed when FSM is entering a state nevertheless which was previous state. Par example this state may contain code to light the lamps in the room. It is not important where was FSM before. The lamps must be lightened on entering the room. Similarly, exit_action may have code to turn off the lights on exitting the room, and it is not important where (in which state) FSM will go.

## Transitions

Transitions do two things - change the FSM state and produce output by executing an action. In contrast of states and events, transitions do not have ```enum``` types. They are represented by type ```sm_transition_t``` and are stored in arrays pointed to by ```const sm_transition_t* tr``` from ```sm_state_t``` objects. Here is the type of transitions:

```c
struct sm_transition {
    sm_event_type_t event;    // trigger event
    sm_state_idx s2;          // destination state (index in an array)
    sm_action_t a;            // action (can be NULL: no action)
    int ai;                   // action index
    sm_guard_t guard;         // guard: true: transition is permitted, false: transition is forbidden;
                              // (can be NULL, then transition is permitted
    bool gpol;                // guard polarity: SM_GPOL_POSITIVE or SM_GPOL_NEGATIVE
};
typedef struct sm_transition sm_transition_t;
```

Meaning of the fields

- `event` = this is triggering event. When incoming event is equal to this value, the transition becomes a candidate fo execution.
- `s2` - the new state. The value is one of the values in the above ```enum```, containing state names. Please remember that this ```enum``` is defined by the application programmer.
- `a` - this is a pointer tu a function (action) that will be executed if the transition is selected for execution
- `ai` - action index. It has supplemental role. It is not used by the FSM, however serves as index in arrays with action names that can be used in FSM tracers. More about this in the sections of FSM tracing.
- `guard` - this a pointer to a function that return ```true``` or ```false```. Returning ```true```  means that the guard permits the execution of the transition, and ```false``` means that it does not permit the transition.
- `gpol` - this is a flag that may or may not negate the result if the guard. gpol may have values of ```SM_GPOL_POSITIVE``` or ```SM_GPOL_NEGATIVE```.

The final result of the guard is result of this expression: ```guard() ^ gpol```. ```SM_GPOL_POSITIVE``` leaves the gaurd result as is. ```SM_GPOL_NEGATIVE``` turns ```true``` to ```false``` and ```false``` to ```true```. This possibility is used to embed non-determinism: Two transitions may be defined with the same triggering event, but with different polarities.

## State machine object

The state machine has a single object describing it, and it has to be in RAM, not in NVM. It is initialized in runtime so as to point to states array. Here is its type definition:

```c
// masks for .flags
#define SM_ACTIVE   0b00000001u // the state machine is active (accepts events)
#define SM_TREN     0b00000010u // true: found transition is permitted by tr->guard
#define SM_TRACE    0b10000000u // tracing, if compiled, is enabled

struct sm_machine {
    sm_state_idx s1;            // current state fo state machine (index)
    int id;                     // state machine identifier (must be unique in the system)
    int flags;                  // internal flags see masks for .flags above
    sm_event_type_t ev;         // incoming event being handled
    const sm_state_t* states;   // array of states, s1 is index for it
    int sizes;                  // number of states in states[]
    void* ctx;                  // pointer to struct containing context information for SM
                                // can be NULL
#if defined(SM_TRACER)
    sm_transition_tracer_t trm;   // pointer to tracer of the state machine
    sm_context_tracer_t trc;      // pointer to context tracer
    sm_lostevent_tracer_t trle;   // pointer to tracer of of lost events
#endif  // defined(SM_TRACER)
};
typedef struct sm_machine sm_machine_t;
```

The comments in the structure describe the fields.

## Context

The context pointer ```ctx``` is an optional. If not used it shall to be ```NULL```. It may be used to point to an application defined structure that contains data used by the actions. This contexts can be useful when a given state machine has several instances and every instance has its own data. Actions can access the context via their parameter ```sm_machine_t* machine``` pointer: ```this->ctx```. Of cource, ```(this->ctx)``` has to be casted to some application defined type.

# Trace

FSM has embedded tracing capabilities. There are three trace functions that (1) are pointed to by poiners in ```sm_machine_t``` object and (2) they are supplied by the application program. Tracers are provided in this demo project and can be used to develop appropriate tracers in real applications. The tracers are pointed by

Trace capabilities are compiled if ```SM_TRACER``` preprocessor constant is defined. Without it, no trace functions are compiled however the production code may become smaller and faster. If tracing is compiled, it can be enabled or disabled (default option) in runtime event in the actions of the state machine.

FSM must be given pointers to the tracers befor tracing to take effect. This is performed by ```SM_SetTracers()``` function.

## Trace state machine

State machine tracer produces output about FSM transitions. It is of type ```sm_transition_tracer_t```. It receives two parameters - ```sm_machine_t* machine``` and a pointer to the current transition being executed. The tracer is executed after the current state is exited, transition actrion is executed and before the new state is entered. See the demo project for a sample of transition tracer. Example output of such tracer could be:

```
ID=0001, S1=sP1_F4, S2=sP1_F2, Event=evBTN3Released, Action=P1a3 permitted
ID=0001, S1=sP1_RESOLVE, S2=sP1_F1, Event=evP1Trigger1, Action=P1a1 not permitted
```

where
 - ID is the ```id``` of the state machine
 - S1 is the current state (being exited)
 - S2 is the new state (being entered)
 - Event is the event that has triggered the transition
 - Action is the action being performed in the transition
 - "permitted" or "not permitted" marks tell whether the action has been permitted to be executed or not.

As it can be seen, this ouput gives information how FSM logic works at its level of abstraction.

## Trace context

It is normally to be traced the context - the application data and how it has been changed. This tracing is what context tracer does. FSM module normally does not know much of the application data. Even, it does not know anything. So the context tracer is to be created by the application programmer. The context tracers receive as arguments a pointer to the state machine and a flag which says when it is called: Context tracers are called twice in one transition - before transition execution and after transition execution. Thorough ```this``` pointer, the void pointer ```this->ctx``` gives access to context data, connected to the FSM instance. It is up to the tracer (and its creator) to cast this void pointer to something meaningful and to retrieve data. Remember that that FSM actions receive ```this`` pointer and via ```this->ctx``` can manipulate context data.

## Trace lost events

Lost events are these events that do not trigger any transition in the current state of FSM. It is normal to have lost events because an instance of FSM in given state reacts to a subset of all possible events. While this is normal behavior in production, tracing of lost event may be useful in development phase of a project. Lost events may show when FSM does not react to events while it has to.

Tracers of lost events are to be left to application programmers. They receive a pointer ```this```. ```this->ev``` is the event being traced.

It may be useful to trace not all lost events but events of interest only.

# Implementation. Functions.

All functions receive a pointer to FSM object of type ```sm_machine_t* machine```. This makes FSM module object oriented.

## sm_machine

```c
void sm_machine(sm_machine_t* machine, sm_event_type_t ev);
```

## sm_initialize

```c
void sm_initialize (sm_machine_t* machine, sm_state_idx s1, int id, const sm_state_t* states, int sizes, void* ctx);
```

```sm_initialize``` initializes FSM object. It must be the first function that is executed with FSM.

Parameters:
- ```machine``` - pointer to FSM object
- ```s1``` - initial state of FSM
- ```id``` - FSM identifier
- ```states``` - pointer to an array of states
- ```sizes``` - size of the arrays of the states (the numbr of states)
- ```ctx``` - pointer to FSM context, may be ```NULL```.

## sm_get_id

```c
int sm_get_id (sm_machine_t* machine);
```

```sm_get_id``` return FSM ```id``` property. This can be used par example by tracers.

## sm_get_current_state

```c
sm_state_idx sm_get_current_state (sm_machine_t* machine);
```

```sm_get_current_state``` return the current state of FSM. This function may be used when the current state must be known (in actions). Normally the actions should not be interested of which is current state. The need for knowing the current state may indicate bad FSM design.

## sm_set_current_state

```c
bool sm_set_current_state (sm_machine_t* machine, sm_state_idx s1);
```

```sm_set_current_state``` sets current state of FSM. This function is normaly not used. FSM must follow its transitions to change its states. However non-standard implementations may use it to change the state. This function is not recommended to be used.

## sm_get_state_count

```c
int sm_get_state_count (sm_machine_t* machine);
```

```sm_get_state_count``` returns the number of the states of FSM.

## sm_set_context

```c
void sm_set_context (sm_machine_t* machine, void* ctx);
```

```sm_set_context``` sets the context of FSM. ```ctx``` becomes value of ```machine->ctx```. ```ctx``` may be ```NULL```.

## sm_get_context

```c
void* sm_get_context (sm_machine_t* machine);
```

```sm_get_context``` returns a pointer to the context of FSM. This is the value of ```machine->ctx```.

## sm_start

```c
void sm_start (sm_machine_t* machine, sm_state_idx s1);
```

```sm_start``` activates FSM pointed by ```machine``` and sets its state to ```s1```. If FSM is already active, this function does nothing. FSM becomes ready to waits for events.

## sm_start_with_event

```c
void sm_start_with_event (sm_machine_t* machine, sm_state_idx s1, sm_event_type_t ev);
```

```sm_start_with_event``` activates FSM pointed by ```machine```, sets its state to ```s1``` and pushes into event buffer the event ```ev```. If FSM is already active, this function does nothing. Pushing an event makes FSM begin working immediately.

## sm_activate

```c
void sm_activate (sm_machine_t* machine);
```

```sm_activate``` enables FSM pointed to by ```machine```.

## sm_deactivate

```c
void sm_deactivate (sm_machine_t* machine);
```

```sm_deactivate``` disables FSM pointed to by ```machine```.

## sm_is_activated

```c
bool sm_is_activated (sm_machine_t* machine);
```

```sm_is_activated``` returns the state of FSM: ```true``` if FSM is active (working) and ```false``` otherwise.

## sm_trace_on

```c
void sm_trace_on (sm_machine_t* machine);
```

```sm_trace_on``` enables trace function.

## sm_trace_off

```c
void sm_trace_off (sm_machine_t* machine);
```

```sm_trace_off``` disables trace function.

## sm_trace_lost_event_on

```c
void sm_trace_lost_event_on(sm_machine_t* machine);
```

```sm_trace_lost_event_on``` enables tracing of lost events for given state machine.

## sm_trace_lost_event_off

```c
void sm_trace_lost_event_off(sm_machine_t* machine);
```

```sm_trace_lost_event_off``` disables tracing of lost events for given state machine.

## sm_set_tracers

```c
void sm_set_tracers(sm_machine_t* machine, sm_transition_tracer_t trm, sm_context_tracer_t trc, sm_lostevent_tracer_t trle);
```

```sm_set_tracers``` sets tracers for FSM pointed to by ``machine```. These are
- ```trm``` - transition tracer
- ```trc``` - context tracer
- ```trle``` - lost events tracer

Each of these parameters can be ```NULL``.

## sm_is_trace_enabled

```c
bool sm_is_trace_enabled (sm_machine_t* machine);
```

```sm_is_trace_enabled``` returns the state of trace: ```true``` if trace enabled and ```false``` otherwise.
