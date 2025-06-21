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
  - [SM\_Machine](#sm_machine)
  - [SM\_Initialize](#sm_initialize)
  - [SM\_GetID](#sm_getid)
  - [SM\_GetCurrentState](#sm_getcurrentstate)
  - [SM\_SetCurrentState](#sm_setcurrentstate)
  - [SM\_GetStateCount](#sm_getstatecount)
  - [SM\_SetContext](#sm_setcontext)
  - [SM\_GetContext](#sm_getcontext)
  - [SM\_Start](#sm_start)
  - [SM\_StartWithEvent](#sm_startwithevent)
  - [SM\_Activate](#sm_activate)
  - [SM\_Deactivate](#sm_deactivate)
  - [SM\_IsActivated](#sm_isactivated)
  - [SM\_TraceOn](#sm_traceon)
  - [SM\_TraceOff](#sm_traceoff)
  - [SM\_TraceLostEventOn](#sm_tracelosteventon)
  - [SM\_TraceLostEventOff](#sm_tracelosteventoff)
  - [SM\_SetTracers](#sm_settracers)
  - [SM\_IsTraceEnabled](#sm_istraceenabled)

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

Events are reprsented as values of type ```EVENT_TYPE```. It is defined as an ```enum```. Example:

```c
enum events_list {
    evNullEvent,
// TO DO: insert application events here
	evButtonPressed, evButtonReleased,

    evEventsNumber
};
typedef enum events_list EVENT_TYPE;
```

Events are just values in ```EVENT_TYPE```. The first event, ```evNullEvent``` is a special no-event value (when there is no new event in the input). It has value of zero. All other event which are application specific and must be inserted after ```evNullEvent```. The last element is ```evEventsNumber``` which represents the number of items in the application that the FSM can accept.

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

Each state may be assigned an entry and an exit action. These actions are functions with signature ```SM_ACTION```, that are called on executing transitions between the states. What they will do is up to the application programmer. The states have set (array) of transitions which will be executed on arriving correspondent events. A state may have not transitions. This means that FSM will not react to any event. It looks like the state machine has dead.

Each state have a structure of data. Here it is:

```c
struct sm_state {
    const SM_TRANSITION* tr;    // array of exit transitions (can be NULL: no exit transition from the state)
    int size;                   // number of transitions in tr[]
    SM_ACTION entry_action;     // action on entering state, may be NULL
    SM_ACTION exit_action;      // action on exiting state, may be NULL
};
typedef struct sm_state SM_STATE;
```
The comments explain what the members are. Transitions are explained below. Here are some words about actions. The type ```SM_ACTION``` is defined as follows:

```
typedef void (*SM_ACTION)(SM_MACHINE* machine);
```

This is a pointer to a C function returning nothing and accepting a pointer to the state machine object. ```entry_action``` is executed when FSM is entering a state nevertheless which was previous state. Par example this state may contain code to light the lamps in the room. It is not important where was FSM before. The lamps must be lightened on entering the room. Similarly, exit_action may have code to turn off the lights on exitting the room, and it is not important where (in which state) FSM will go.

## Transitions

Transitions do two things - change the FSM state and produce output by executing an action. In contrast of states and events, transitions do not have ```enum``` types. They are represented by type ```SM_TRANSITION``` and are stored in arrays pointed to by ```const SM_TRANSITION* tr``` from ```SM_STATE``` objects. Here is the type of transitions:

```c
struct sm_transition {
    EVENT_TYPE event;       // trigger event
    STATE_TYPE s2;          // destination state (index in an array)
    SM_ACTION a;            // action (can be NULL: no action)
    int ai;                 // action index
    SM_GUARD guard;         // guard: true: transition is permitted, false: transition is forbidden;
                            // (can be NULL, then transition is permitted
    bool gpol;              // guard polarity: SM_GPOL_POSITIVE or SM_GPOL_NEGATIVE
};
typedef struct sm_transition SM_TRANSITION;
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
    STATE_TYPE s1;              // current state fo state machine (index)
    int id;                     // state machine identifier (must be unique in the system)
    int flags;                  // internal flags see masks for .flags above
    EVENT_TYPE ev;              // incoming event being handled
    const SM_STATE* states;     // array of states, s1 is index for it
    int sizes;                  // number of states in states[]
    void* ctx;                  // pointer to struct containing context information for SM
                                // can be NULL
#if defined(SM_TRACER)
    SM_TRANSITION_TRACER trm;   // pointer to tracer of the state machine
    SM_CONTEXT_TRACER trc;      // pointer to context tracer
    SM_LOSTEVENT_TRACER trle;   // pointer to tracer of of lost events
#endif  // defined(SM_TRACER)
};
typedef struct sm_machine SM_MACHINE;
```

The comments in the structure describe the fields.

## Context

The context pointer ```ctx``` is an optional. If not used it shall to be ```NULL```. It may be used to point to an application defined structure that contains data used by the actions. This contexts can be useful when a given state machine has several instances and every instance has its own data. Actions can access the context via their parameter ```SM_MACHINE* machine``` pointer: ```this->ctx```. Of cource, ```(this->ctx)``` has to be casted to some application defined type.

# Trace

FSM has embedded tracing capabilities. There are three trace functions that (1) are pointed to by poiners in ```SM_MACHINE``` object and (2) they are supplied by the application program. Tracers are provided in this demo project and can be used to develop appropriate tracers in real applications. The tracers are pointed by

Trace capabilities are compiled if ```SM_TRACER``` preprocessor constant is defined. Without it, no trace functions are compiled however the production code may become smaller and faster. If tracing is compiled, it can be enabled or disabled (default option) in runtime event in the actions of the state machine.

FSM must be given pointers to the tracers befor tracing to take effect. This is performed by ```SM_SetTracers()``` function.

## Trace state machine

State machine tracer produces output about FSM transitions. It is of type ```SM_TRANSITION_TRACER```. It receives two parameters - ```SM_MACHINE* machine``` and a pointer to the current transition being executed. The tracer is executed after the current state is exited, transition actrion is executed and before the new state is entered. See the demo project for a sample of transition tracer. Example output of such tracer could be:

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

All functions receive a pointer to FSM object of type ```SM_MACHINE* machine```. This makes FSM module object oriented.

## SM_Machine

```c
void SM_Machine (SM_MACHINE* machine, EVENT_TYPE ev);
```

## SM_Initialize

```c
void SM_Initialize (SM_MACHINE* machine, STATE_TYPE s1, int id, const SM_STATE* states, int sizes, void* ctx);
```

```SM_Initialize``` initialises FSM object. It must be the first function that is executed with FSM.

Parameters:
- ```this``` - pointer to FSM object
- ```s1``` - initial state of FSM
- ```id``` - FSM identifier
- ```states``` - pointer to an array of states
- ```sizes``` - size of the arrays of the states (the numbr of states)
- ```ctx``` - pointer to FSM context, may be ```NULL```.

## SM_GetID

```c
int SM_GetID (SM_MACHINE* machine);
```

```SM_GetID``` return FSM ```id``` property. This can be used par example by tracers.

## SM_GetCurrentState

```c
STATE_TYPE SM_GetCurrentState (SM_MACHINE* machine);
```

```SM_GetCurrentState``` return the current state of FSM. This function may be used when the current state must be known (in actions). Normally the actions should not be interested of which is current state. The need for knowing the current state may indicate bad FSM design.

## SM_SetCurrentState

```c
bool SM_SetCurrentState (SM_MACHINE* machine, STATE_TYPE s1);
```

```SM_SetCurrentState``` sets current state of FSM. This function is normaly not used. FSM must follow its transitions to change its states. However non-standard implementations may use it to change the state. This function is not recommended to be used.

## SM_GetStateCount

```c
int SM_GetStateCount (SM_MACHINE* machine);
```

```SM_GetStateCount``` returns the number of the states of FSM.

## SM_SetContext

```c
void SM_SetContext (SM_MACHINE* machine, void* ctx);
```

```SM_SetContext``` sets the context of FSM. ```ctx``` becomes value of ```this->ctx```. ```ctx``` may be ```NULL```.

## SM_GetContext

```c
void* SM_GetContext (SM_MACHINE* machine);
```

```SM_GetContext``` returns a pointer to the context of FSM. This is the value of ```this->ctx```.

## SM_Start

```c
void SM_Start (SM_MACHINE* machine, STATE_TYPE s1);
```

```SM_Start``` activates FSM pointed by ```this``` and sets its state to ```s1```. If FSM is already active, this function does nothing. FSM becomes ready to waits for events.

## SM_StartWithEvent

```c
void SM_StartWithEvent (SM_MACHINE* machine, STATE_TYPE s1, EVENT_TYPE ev);
```

```SM_StartWithEvent``` activates FSM pointed by ```this```, sets its state to ```s1``` and pushes into event buffer the event ```ev```. If FSM is already active, this function does nothing. Pushing an event makes FSM begin working immediately.

## SM_Activate

```c
void SM_Activate (SM_MACHINE* machine);
```

```SM_Activate``` enables FSM pointed to by ```this```.

## SM_Deactivate

```c
void SM_Deactivate (SM_MACHINE* machine);
```

```SM_Deactivate``` disables FSM pointed to by ```this```.

## SM_IsActivated

```c
bool SM_IsActivated (SM_MACHINE* machine);
```

```SM_IsActivated``` returns the state of FSM: ```true``` if FSM is active (working) and ```false``` otherwise.

## SM_TraceOn

```c
void SM_TraceOn (SM_MACHINE* machine);
```

```SM_TraceOn``` enables trace function.

## SM_TraceOff

```c
void SM_TraceOff (SM_MACHINE* machine);
```

```SM_TraceOff``` disables trace function.

## SM_TraceLostEventOn

```c
void SM_TraceLostEventOn(SM_MACHINE* machine);
```

```SM_TraceLostEventOn``` enables tracing of lost events for given state machine.

## SM_TraceLostEventOff

```c
void SM_TraceLostEventOff(SM_MACHINE* machine);
```

```SM_TraceLostEventOff``` disables tracing of lost events for given state machine.

## SM_SetTracers

```c
void SM_SetTracers(SM_MACHINE* machine, SM_TRANSITION_TRACER trm, SM_CONTEXT_TRACER trc, SM_LOSTEVENT_TRACER trle);
```

```SM_SetTracers``` sets tracers for FSM pointed to by ``this```. These are
- ```trm``` - transition tracer
- ```trc``` - context tracer
- ```trle``` - lost events tracer

Each of these parameters can be ```NULL``.

## SM_IsTraceEnabled

```c
bool SM_IsTraceEnabled (SM_MACHINE* machine);
```

```SM_IsTraceEnabled``` returns the state of trace: ```true``` if trace enabled and ```false``` otherwise.