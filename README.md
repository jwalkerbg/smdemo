# State Machine

SM is an implementation of a event driven finite state machine in C language.

The machine is implemented as a single C function that performs a transition between states (if found suitable one and permitted). States and transitions are stored in tables, so the state machine functions uses these tables to select transitions.

State machine function does not loop. On each call it performs a transition and exits. This means that the applications that use SM must create main loop.

While SM is written in C, it is created object oriented as much as posiible. Most of the functions receive as first parameter a pointer to a structure, that represents the state machine. If one needs, he would convert this code in C++ easily.

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
```
The comments explain what the members are. Transitions are explained below. Here some words about actions. The type ```SM_ACTION``` is defined as follows:

```
typedef void (*SM_ACTION)(SM_MACHINE* this);
```

This is a pointer to a C function returning nothing and accepting a pointer to the state machine object. ```entry_action``` is executed when FSM is entering a state nevertheless which was previous state. Par example this state may contain code to light the lamps in the room. It is not important where was FSM before. The lamps must be lightened on entering the room. Similarly, exit_action may have code to turn off the lights on exitting the room, and it is not important where (in which state) FSM will go.