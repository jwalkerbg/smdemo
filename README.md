# State Machine

SM is an implementation of a event driven finite state machine in C language.

The machine is implemented as a single C function that performs a transition between states (if found suitable one and permitted). States and transitions are stored in tables, so the state machine functions uses these tables to select transitions.

State machine function does not loop. On each call it performs a transition and exits. This means that the applications that use SM must create main loop.

Nevertheless SM is written in C, it is created object oriented as much as posiible. Most of the functions receive as first parameter a pointer to a structure, that represents the state machine. If one needs, he would convert this code in C++ easily.

Stste machine helps seprate the logic from actions. It makes clear separation between input, logic and output. This is essential for creating stable and extendable applications without lossing the full picture.

# Formal definition.

State machine (Finite state machine - FSM) is an abstract machine that has associated with it finite number of states. It can be in exactly one of the states at any given time. The FSM change from one state to another in responce of some input. The set of inputs which this FSM reacts is set of abstract events. Every change from one to another state is a transition. Is is allowed cuurent state and next stte to be thesam state. On transitions, the state machine may produce output. The output of this FSM are actions. On each transition the FSM performs actions.

Transitions have guards. Guards are logical functions that permit or not permit transitions to be executed. Guards add nondeterministic character ot the FSM. Without them the machine is fully deterministic.

# Implementation

## Events

Events are reprsented as values of type ```EVENT_TYPE```. It is defined as an ```enum```.

```
enum events_list {
	evNullEvent,
// TO DO: insert application events here

    evEventsNumber	
};
typedef enum events_list EVENT_TYPE;
```

Events are just values in ```EVENT_TYPE```.The first event, ```evNullEvent``` is a special no-event value (when there is no new event in the input). It has value of zero. All other event which are application specific must be inserted after ```evNullEvent```. The last element is ```evEventsNumber``` which represents the number of items in the application that the FSM can accept.

The names of application specific event must b prfixed with ```ev``` prefix to avoid coflicts with values of other enums that may be defined in the application.

Events in the type cannot have gaps. This means that ```evSomeEvent = 10``` is not allowed. The events cannot produce equal integer values.

The names of the event shall represent its semantic. Example: ```evDoorOpened```, ```evDoorClosed``` witch may originate from some door sensor. ```evMeasurementsReady``` may originate from an low level algorithm that measures analog levels of some analog (ADC) inputs. Events usually represent changes of states of some sub-systems in the application. Taking above name ```evDoorOpened``` means that the door has been closed (initial state) and it is opened now (the new state). The event tells that it was opened somehow - by a person, by the wind and so on.

## States

States in SM have individual codes, that are presented by ```enums```. It is convenient that each state machine in the applicaion to have its own definition. And example:

```
enum sP1_states {
    sP1_START, sP1_RESOLVE, sP1_F1, sP1_F2, sP1_F3, sP1_F4,

    sP1_StatesNumber
};
```

Here, the naming is as follows:

- ```s``` - indicates state
- ```P1``` - is the name of the state machine - par example it implements the behaviour of an application process / task / thread, named ```P1```.
- ```START```, ```RESOLVE``` rtc are the names of the states. These are selected / created by the application programmer. Values in these enums serve as index in the array of states, associated with the state machine (see below).

Note that it is good idea the last item in an enum tobe a special key which is repersent the total number of states.

Each state may be assigned an entry and an exit action. These actions are functions, that are called on executing a transition between the states. What they will do is up to the application programmer.The states have set (array) of transitions whch will be executed on arriving correspondent event. A state may have not transitions. This means that FSM will not react to any event. It looks like the state machine has dead.

Each state have a structure of data. Here it is:

```
struct sm_state {
    const SM_TRANSITION* tr;    // array of exit transitions (can be NULL: no exit transition from the state)
    int size;                   // number of transitions in tr[]
    SM_ACTION entry_action;     // action on entering state, may be NULL
    SM_ACTION exit_action;      // action on exiting state, may be NULL
};
typedef struct sm_state SM_STATE;
```
The comments explain what the members are. Transitions are explained below. Here some words about actions. The type ```SM_ACTION``` is defined as follows:

```
typedef void (*SM_ACTION)(SM_MACHINE* this);
```

This is a pointer to a C function returning nothing and accepting a pointer to the state machine object. ```entry_action``` is executed when FSM is entering a state nevertheless which was previous state. Par example this state may contain code to light the lamps in the room. It is not important where was FSM before. The lamps must be lightened on entering the room. Similarly, exit_action may have code to turn off the lights on exitting the room, and it is not important where (in which state) FSM will go.

## Transitions

Transitions do two things - change the FSM state and produce output by calling an action. In contrst fo states and events, transitions do not have ```enum``` types. They are represented by type ```SM_TRANSITION``` and are stored in arrays pointed to by ```const SM_TRANSITION* tr``` from ```SM_STATE``` objects. Here is the type of transitions:

```
struct sm_transition {
    EVENT_TYPE event;       // trigger event
    STATE_TYPE s2;          // destination state (index in an array)
    SM_ACTION a;            // action (can be NULL: null action)
    int ai;                 // action index
    SM_GUARD guard;         // guard: true: transition is permitted, false: transition is forbidden;
                            // (can be NULL, then transition is permitted
    bool gpol;              // guard polarity: SM_GPOL_POSITIVE or SM_GPOL_NEGATIVE
};
typedef struct sm_transition SM_TRANSITION;
```

Meanung of the fields

- event = this is triggering event. When incoming event is equal to this value, the transition becomes a candidate fo execution.
- s2 - the new state. The value is one of the values in the above ```enum```, containing state names. Please remember that this ```enum``` is defined by the application programmer.
- a - this is a pointer tu a function (action) that will be executed if the transition is selected for execution
- ai - action index. It has supplemental role. It is not used by the FSM, however serves as index in arrays with action names that can be used in FSM tracers. More about this in the sections of FSM tracing.
- guard - this a pointer to a function that return ```true``` or ```false```. Returning ```true```  means that the guard permits the execution of the transition, and ```false``` means that it does not permit the transition.
- gpol - this is a flag that may or may not negate the result if the guard. gpol may have values of ```SM_GPOL_POSITIVE`` or ```

The fnal result of the guard is result of this expression: ```guard() ^ gpol```. ```SM_GPOL_POSITIVE``` leaves the gaurdc result as is. ```SM_GPOL_NEGATIVE``` turns ```true``` to ```false``` and ```false``` to ```true```. This possibility is usedto embed non-determinism: Two transitions may be defined with the same triggering event, but with different polarities.

## State machine object

The state amchine has a single object describing it, and it has to be in RAM, not in NVM. It is initializwed in runtime so as to point to states array. Here is its type definition 

```
struct sm_machine {
    STATE_TYPE s1;              // current state fo state machine (index)
    int id;                     // state machine identifier (must be unique in the system)
    int flags;                  // internal flags see masks for .flags above
    EVENT_TYPE ev;              // active event been handled
    const SM_STATE* states;     // array of states, s1 is index for it
    int sizes;                  // number of states in states[]
    void* ctx;                  // pointer to struct containing context information for SM
                                // can be NULL
#if defined(SM_TRACER)
    SM_TRANSITION_TRACER trm;   // pointer to tracer of the state machine
    SM_CONTEXT_TRACER trc;      // pointer to context tracer
    SM_LOSTEVENT_TRACER trle;   // tracer of of lost events
#endif  // defined(SM_TRACER)
};
typedef struct sm_machine SM_MACHINE;
```

The comments in the struture describe the fields. 

