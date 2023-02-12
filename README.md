# State Machine

SM is an implementation of a event driven finite state machine in C language.

The machine is implemented as a single C function that performs a transition between states (if found suitable one and permitted). States and transitions are stored in tables, so the state machine functions uses these tables to select transitions.

State machine function does not loop. On each call it performs a transition and exits. This means that the applications that use SM must create main loop.

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

The names of application specific event must b prfixed with ````ev``` prefix to avoid coflicts with values of other enums that may be defined in the application.

Events in the type cannot have gaps. This means that ```evSomeEvent = 10``` is not allowed. The events cannot produce equal integer values.

The names of the event shall represent its semantic. Example: ```evDoorOpened```, ```evDoorClosed``` witch may originate from some door sensor. ```evMeasurementsReady``` may originate from an low level algorithm that measures analog levels of some analog (ADC) inputs. Events usually represent changes of states of some sub-systems in the application. Taking above name ```evDoorOpened``` means that the door has been closed (initial state) and it is opened now (the new state). The event tells that it was opened somehow - by a person, by the wind and so on.