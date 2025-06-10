- [FSM introduction.](#fsm-introduction)
- [Overview.](#overview)
  - [What is a Finite State Machine?](#what-is-a-finite-state-machine)
  - [Core Components](#core-components)
  - [Types of FSMs.](#types-of-fsms)
  - [Example (Traffic Light FSM)](#example-traffic-light-fsm)
  - [Applications.](#applications)
  - [Where FSMs Fit in Software Architecture](#where-fsms-fit-in-software-architecture)
    - [Typical Software Architecture Levels (Top to Bottom)](#typical-software-architecture-levels-top-to-bottom)
    - [Benefits of This Approach.](#benefits-of-this-approach)
  - [Advanced Enhancements (Optional Ideas)](#advanced-enhancements-optional-ideas)
  - [Summary View](#summary-view)
- [Events and coupling](#events-and-coupling)
  - [The Architecture: Key Characteristics](#the-architecture-key-characteristics)
    - [Diagram: Conceptual Flow](#diagram-conceptual-flow)
  - [Advantages of The Design](#advantages-of-the-design)
    - [Central Event Dispatcher](#central-event-dispatcher)
    - [Event Types \& Filtering](#event-types--filtering)
    - [FSM Interface Standardization](#fsm-interface-standardization)
      - [FSM-Generated Events](#fsm-generated-events)
    - [Missed Events Handling (Optional Logging)](#missed-events-handling-optional-logging)
    - [Summary](#summary)
  - [Physical and abstract events.](#physical-and-abstract-events)
    - [Event Abstraction Model](#event-abstraction-model)
    - [This is Excellent for Multiple Reasons](#this-is-excellent-for-multiple-reasons)
      - [Loose Coupling via Abstraction](#loose-coupling-via-abstraction)
      - [Unidirectional, Clear Flow of Control](#unidirectional-clear-flow-of-control)

# FSM introduction.

This document presents a small talk with ChatGPT about Finite State Machines (FSM). It gives fundamental information about FSM - what they are, what they are used for etc. Happy reading.

# Overview.

## What is a Finite State Machine?

A **Finite State Machine (FSM)** is a computational model consisting of a finite number of **states**, **transitions** between those states, and **actions**. An FSM can only be in **one state at a time**, and it transitions to other states in response to **inputs** or **events**.

## Core Components

1. **States**: Represent different modes or configurations the system can be in.
2. **Initial State**: The state the machine starts in.
3. **Events/Inputs**: External triggers that cause transitions.
4. **Transitions**: Rules that determine how to move from one state to another based on events.
5. **Actions**:
   1. **Entry Actions**: Executed upon entering a state.
   2. **Exit Actions**: Executed when leaving a state.
   3. **Transition Actions**: Executed during the transition between states.

## Types of FSMs.

1. **Deterministic FSM (DFSM)**: For a given state and input, the next state is uniquely determined.
2. **Non-deterministic FSM (NFSM)**: For a given state and input, multiple transitions may be possible.
3. **Moore Machine**: Output depends only on the current state.
4. **Mealy Machine**: Output depends on the current state and input.

## Example (Traffic Light FSM)

States: `Red`, `Green`, `Yellow`

Transitions:

* `Red` → `Green` on `timer`
* `Green` → `Yellow` on `timer`
* `Yellow` → `Red` on `timer`

## Applications.

* Embedded systems (e.g. washing machines, elevators)
* Network protocols
* Game AI
* User interface logic
* Lexical analyzers

## Where FSMs Fit in Software Architecture

FSMs usually sit at the **mid-to-upper levels** of a software architecture — they act as **behavioral controllers** for the system or subsystems. Your description of having a main FSM and several specific-purpose FSMs is a **modular layered FSM architecture**, which is well-suited to complex embedded designs.

Usually, several FSMs work on top leve, one of them is the m,ain FSM and others implement specific aspects of functionality. Below FS levels are procedures, devices drivers etc.

### Typical Software Architecture Levels (Top to Bottom)

Here's a rough breakdown of layers in embedded or control-based software systems, and where FSMs typically sit:

```
Application Layer (Top)
│
├── FSM Layer
│   ├── Main FSM (System orchestrator)
│   ├── Sub-FSMs (Feature controllers, UI, modes, communication, etc.)
│
├── Service Layer (Middleware)
│   ├── Protocol handlers, data processing, utility logic
│
├── Hardware Abstraction Layer (HAL)
│   ├── Drivers for peripherals, RTOS interfaces
│
└── Hardware (Bottom)
```

The above structure:

* **Main FSM**: Orchestrates overall system modes or flow (e.g. startup, normal operation, fault).
* **Sub-FSMs**: Handle well-scoped responsibilities (e.g. communication protocol handling, UI states, peripheral control).
* **Procedures/Drivers**: Implement below the FSMs as services/utilities that are stateless or loosely stateful.

This matches a **separation of concerns** pattern:

* **FSMs → decision-makers (when and what to do)**
* **Procedures/drivers → doers (how to do it)**

### Benefits of This Approach.

* **Modularity**: Sub-FSMs encapsulate logic for specific functions
* **Testability**: FSMs can be tested independently of drivers
* **Scalability**: Adding features just means adding or extending FSMs
* **Maintainability**: Logic and behavior are cleanly separated from low-level details

## Advanced Enhancements (Optional Ideas)

* **Hierarchical FSMs (HFSMs)**: A sub-FSM can be nested inside a state of a parent FSM (common in statecharts/UML).
* **FSM Frameworks**: Consider using or building a lightweight FSM framework with features like event queues, timers, or state history.
* **Event Bus**: Use a central dispatcher/event bus for loosely-coupled FSM interaction.

## Summary View

Layer | Role | FSM Involvement
------|------|-----------------
Application | High-level goals, UI, modes | ✅ Main & sub FSMs
Behavioral Controllers | Decision logic | ✅ FSM core
Middleware/Services | Supportive logic | 🚫 Often stateless
Drivers / HAL | Hardware access | 🚫 Procedural
RTOS / Hardware | Scheduling, interrupts | 🚫 Outside FSM scope

# Events and coupling

About events - they are generated by the low level modules -user IO, sensors, etc. Also FSMs can generate events and this way to communicate with other FSMs. Every event is sent to each of the active FSM so as the FSM to try to find a transition to execute. If not found, the FSM is said that it miss the event without action.

This approach is very sound and clean, especially for embedded or complex real-time systems. This is a loosely-coupled, event-driven FSM architecture, where FSMs operate independently but interact through a shared event distribution mechanism. Let me reflect this model, break it down, and point out the strengths and possible refinements.

## The Architecture: Key Characteristics

Aspect | Design
-------|-------
FSM Coupling | Loosely coupled
Event Sources | Low-level modules (IO, sensors), and FSMs themselves
Event Handling | Events are broadcast to all active FSMs
FSM Transition Logic | FSMs handle events independently, ignore if not relevant
FSM Communication | FSMs emit events to signal others (indirect interaction)

This is essentially an **event bus/broadcast pattern** with **selective consumption** by FSMs — a great pattern for separation of concerns and modularity.

### Diagram: Conceptual Flow

```
[User IO]      [Sensor]        [Timer]        [FSM-A]
   ↓              ↓               ↓              ↓
            ---> Event Queue / Dispatcher ---> Broadcast
                                            ↙     ↓     ↘
                                       [FSM-A] [FSM-B] [FSM-C]
                                      (handles?) (handles?) ...
```

## Advantages of The Design

* ✅ Modularity: FSMs can be developed/tested independently
* ✅ Extensibility: New FSMs can subscribe to events without changing others
* ✅ Loose coupling: FSMs don't need to know about each other
* ✅ Natural concurrency: FSMs react to events independently

** Event Processing: Options and Observations

How each FSM processes events:

* On receiving an event, the FSM checks whether the current state allows a transition on that event.
* If no transition is defined for the event → "missed" (harmless, no side effects).
* Possibly logs or counts missed events for diagnostics.

This is a reactive system that promotes **simple FSMs** — each only needs to know about the events it cares about.

** Suggestions to Enhance or Structure This System

Here are a few refinements you might find useful:

### Central Event Dispatcher

Encapsulate event delivery into a dispatcher or event bus:

```c
void dispatch_event(Event e) {
    for (int i = 0; i < active_fsm_count; i++) {
        fsm_handle_event(active_fsms[i], e);
    }
}
```

You might also:

* Prioritize events
* Queue them if real-time pressure requires it
* Allow event filtering per FSM if optimization is needed

### Event Types & Filtering

Define a strong `EventType` enum or bitfield. FSMs could declare interest in specific types, reducing overhead:

```c
typedef enum {
    EVENT_BUTTON_PRESS,
    EVENT_TEMP_THRESHOLD,
    EVENT_COMM_RECEIVED,
    ...
} EventType;
```
### FSM Interface Standardization

Make each FSM expose a uniform interface:

```c
typedef struct {
    const char* name;
    void (*handle_event)(FSM*, Event);
    void (*enter_state)(FSM*, State);
    State current_state;
} FSM;
```

This makes dispatching, logging, debugging easier.

#### FSM-Generated Events

Events generated by FSMs to others = powerful! This creates a **decoupled chain of behavior**.

Example: FSM-A handles a sensor threshold, emits `EVENT_OVERHEAT`, FSM-B (cooling) responds by starting a fan.

### Missed Events Handling (Optional Logging)

Even though "misses" are expected, it can be useful to log:

* Unexpected events
* Rate of "no-op" events (helps debug why nothing’s happening)

### Summary

This model is a essentially lightweight actor model, where FSMs are actors reacting to events:

    🟢 Each FSM is autonomous

    🟡 Events are shared but selectively handled

    🔵 FSMs communicate indirectly through events

    🔴 Missing an event is not an error — it's by design

It’s a **robust and scalable pattern** that works well in both bare-metal and RTOS-based systems.

## Physical and abstract events.

It is good idea to convert miscellaneous physical events to some mathematical abstract representation like integers from an C enum type. Then this absolutely abstract values (that do not depend on the character of the real events like temperature changes, pressure changes, voltage changes and so on) are passed to the FSMs. They do not know anything about the real events, however they know how to handle the abstract events - to make or not to make a transition.

Then if a transition is performed, it executes an action. These actions return the execution to the "real world". Actions may call procedures in the middle ware to open/close doors, to start motors, to light LEds etc. Also actions can generate other abstract events to make the logic work and to communicate between the FSMs.

This model of using events and converting physical events to abstract and vice versa is a strong grasp of abstraction, modularity, and separation of concerns. This kind of design is ideal in embedded and complex control systems. Let's unpack it, validate your reasoning, and add a few comments and potential refinements.

### Event Abstraction Model

This is a event normalization:

    Physical Events (sensor triggers, button presses, voltage changes)
    ↓

    Abstract Events (enum values, integers)
    ↓

    FSMs consume these abstract events to make decisions
    ↓

    Actions (on transitions) bring decisions back into the real world
    ↓

    Side effects (motor control, LEDs, messages, or generating more abstract events)


### This is Excellent for Multiple Reasons

#### Loose Coupling via Abstraction

FSMs no longer care **what caused the event** — they only react to **what the event means** (in the logic of the system). This eliminates tight coupling between logic and input sources.

Example:
    * `EVENT_ALARM_TRIGGERED` might result from over-temperature, a door open too long, or a vibration sensor — but the FSM doesn't need to know which.
    * You can swap sensors or fuse inputs without modifying the FSM.

#### Unidirectional, Clear Flow of Control

We reach separation:

    * **Inputs** → **Abstract Events**
    * **FSMs** → **Decision-making (pure logic)**
    * **Actions** → **Side effects / "back to reality"**

This makes each part:

    * **Simpler**
    * **Testable in isolation**
    * **Reusable**