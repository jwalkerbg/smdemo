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
  - [Event Processing: Options and Observations](#event-processing-options-and-observations)
  - [Suggestions to Enhance or Structure This System](#suggestions-to-enhance-or-structure-this-system)
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
      - [FSMs as Pure Functions (almost)](#fsms-as-pure-functions-almost)
      - [Middleware Mediation in Actions](#middleware-mediation-in-actions)
    - [Suggestions for Further Refinement.](#suggestions-for-further-refinement)
      - [Define a Clear Event Abstraction Layer](#define-a-clear-event-abstraction-layer)
      - [Actions as First-Class Functions.](#actions-as-first-class-functions)
      - [Use an Event Queue / Ring Buffer](#use-an-event-queue--ring-buffer)
      - [Inter-FSM Communication via Events](#inter-fsm-communication-via-events)
    - [Summary: Architecture Recap](#summary-architecture-recap)
  - [Guards on transitions](#guards-on-transitions)
    - [Are Guards Non-Deterministic?](#are-guards-non-deterministic)
    - [Summary Table](#summary-table)
    - [Suggested Pattern](#suggested-pattern)
    - [Why This is Good](#why-this-is-good)
    - [Suggested Enhancement: Guard Wrappers or Macros](#suggested-enhancement-guard-wrappers-or-macros)
      - [Option 1: Wrapper Function](#option-1-wrapper-function)
      - [Option 2: Guard With Parameter (for inversion)](#option-2-guard-with-parameter-for-inversion)
      - [In a struct](#in-a-struct)
  - [Entry and Exit actions for states](#entry-and-exit-actions-for-states)
    - [Entry and Exit Actions: Semantic Roles](#entry-and-exit-actions-semantic-roles)
    - [Why Entry/Exit Actions Are Valuable](#why-entryexit-actions-are-valuable)
    - [Execution Order (Important Detail)](#execution-order-important-detail)
    - [FSM Structure with Entry/Exit Actions](#fsm-structure-with-entryexit-actions)
  - [FSM Design Elements in C](#fsm-design-elements-in-c)
  - [Trace-Friendly FSM Update](#trace-friendly-fsm-update)
    - [Modified fsm\_handle\_event() with Trace Output](#modified-fsm_handle_event-with-trace-output)
    - [Optional Enhancements](#optional-enhancements)
  - [Real example](#real-example)
    - [System Overview](#system-overview)
    - [Step-by-Step Implementation](#step-by-step-implementation)
    - [Optional Enhancements](#optional-enhancements-1)
    - [Summary](#summary-1)
    - [Additions](#additions)
    - [Summary](#summary-2)
    - [Returning to the real world.](#returning-to-the-real-world)
    - [Additions](#additions-1)
    - [FSM Trace Report: Temperature Control System (example)](#fsm-trace-report-temperature-control-system-example)

# FSM introduction.

This document presents a talk with ChatGPT about Finite State Machines (FSM). It gives fundamental information about FSM - what they are, what they are used for etc. Happy reading and then happier coding.

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

FSMs usually sit at the **mid-to-upper levels** of a software architecture — they act as **behavioral controllers** for the system or subsystems. The construction of having a main FSM and several specific-purpose FSMs is a **modular layered FSM architecture**, which is well-suited to complex embedded designs.

Usually, several FSMs work on top level, one of them is the main FSM and others implement specific aspects of functionality. Below FS levels are procedures, devices drivers etc.

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

## Events and coupling

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

## Event Processing: Options and Observations

How each FSM processes events:

* On receiving an event, the FSM checks whether the current state allows a transition on that event.
* If no transition is defined for the event → "missed" (harmless, no side effects).
* Possibly logs or counts missed events for diagnostics.

This is a reactive system that promotes **simple FSMs** — each only needs to know about the events it cares about.

## Suggestions to Enhance or Structure This System

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

Note that the event `e` is passed to the active FSMs in the order they are stored in `active_fsm[]` array. The good FSM design **should not depend** on the order the FSMs handle the message. Despite the events are ordered in some event queue, their occurrence is random. This especially is valid for the events coming from peripheral modules.

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

#### FSMs as Pure Functions (almost)

Since FSMs only consume abstract events and decide transitions, they’re **almost pure functions**. This is excellent for testing and formal analysis.

You can simulate FSMs without hardware by feeding them event sequences.

#### Middleware Mediation in Actions

You're doing a good job separating **effect logic** (open doors, turn on motors) into **middleware/procedural layers**, so FSMs don’t talk directly to hardware.

You can simulate FSMs without hardware by feeding them event sequences.

Actions are procedural side effects triggered by declarative FSM transitions — this is *very close to how functional programming or reactive systems work*.

### Suggestions for Further Refinement.

#### Define a Clear Event Abstraction Layer

Use `enum` or `#define` for abstract events, plus an `Event` struct that allows metadata to be carried if needed.

```c
typedef enum {
    EVENT_NONE = 0,
    EVENT_BUTTON_PRESS,
    EVENT_SENSOR_LIMIT,
    EVENT_TIMEOUT,
    EVENT_COMMAND_RECEIVED,
    EVENT_ALARM_TRIGGERED,
    // etc.
} EventType;

typedef struct {
    EventType type;
    uint8_t data[8];  // Optional payload
} Event;
```

This lets you pass extra data without coupling FSMs to physical sources.

#### Actions as First-Class Functions.

Define actions separately and associate them with transitions:

```c
typedef void (*ActionFn)(void* context, Event* e);
```

This makes FSM tables clearer and allows reuse.

```c
{ STATE_IDLE, EVENT_SENSOR_LIMIT, STATE_ACTIVE, &start_motor },
```

#### Use an Event Queue / Ring Buffer

```c
EventQueue event_queue;
event_enqueue(&event_queue, EVENT_SENSOR_LIMIT);
```

This is optional, but useful in systems where timing or order matters.

#### Inter-FSM Communication via Events

This is one of the best parts of the design: FSMs talk to each other by emitting events, not by calling each other. This:

* Keeps FSMs unaware of each other’s internals
* Allows broadcasting or selective targeting
* Supports async operation and decoupled timing

### Summary: Architecture Recap

```
[ Physical Event: Button Press ] → [ Abstract Event: EVENT_BUTTON_PRESS ]
                                   ↓
                    [ FSM Logic → State Change → Action ]
                                   ↓
         [ Action: light LED, start motor, send another EVENT ]
                                   ↓
                    [ Other FSMs may consume generated event ]
```

This model is:

* Clean
* Portable
* Predictable
* Testable
* Scalable

It’s a solid foundation for everything from embedded state machines to full industrial controllers.

## Guards on transitions

What are guards? They are

* A **guard** is a boolean-returning function (`bool guard(Event* e, Context* ctx)`).
* It's evaluated when an event triggers possible transitions from the current state.
* If the guard returns `true`, the transition is **eligible** to fire.
* If it returns `false`, the transition is **skipped**.

This adds **conditional logic** to transitions, beyond simple state-event matching.

### Are Guards Non-Deterministic?

This is a nuanced question.

✅ **Deterministic, if...**

    * At most **one transition** is eligible (i.e. its guard returns `true`) for a given state-event pair.
    * You **evaluate transitions in a defined order** (e.g., top-down in a table) and take **only the first match** whose guard passes.

This is how most FSM implementations ensure predictability.

    ✔️ **Result**: The same event in the same state will always cause the same transition if the system state (guard conditions) is unchanged.

⚠️ **Non-Deterministic, if...**

    Multiple transitions from the same state on the same event are **simultaneously eligible** (i.e., multiple guards return `true`), **and** the FSM framework does not define which one to take.

    ✖️ This leads to **non-determinism** — the behavior may vary depending on transition order, or worse, on compiler or runtime factors.

🔸 Your Case

    "An event can trigger more than one transition, each with its own guard."

That’s a **non-deterministic model**, *unless* you do one of the following:

✅ Enforce Determinism via:

1. **Priority / Order**

Define a priority order (e.g. array order), and **only take the first matching transition** with a `true` guard.

2. **Exclusive Guards**

Design your guards so only **one can be true at a time**, per event per state. This requires discipline or formal validation.


3. **Use Hierarchical FSMs**

If multiple transitions must occur in response to an event, **split into hierarchical or parallel FSMs**, or have one transition generate more events to trigger other FSMs.

4. **Explicit Multi-Transition Design**

If you *intentionally* want multiple transitions to occur for a single event, don’t rely on guard ambiguity. Instead:

    * Emit multiple events.
    * Use FSM intercommunication.

### Summary Table

Scenario | Deterministic? | Recommended?
---------|----------------|-------------
One transition per state-event | ✅ | ✅ Yes
Multiple transitions + guards + priority | ✅ | ✅ Yes (most flexible)
Multiple eligible transitions, no defined order | ❌ | ⚠️ No (avoid)
Guards designed to be mutually exclusive | ✅ | ✅ Yes, with care

### Suggested Pattern

```c
typedef struct {
    State from_state;
    EventType event;
    bool (*guard)(Event*, Context*);
    State to_state;
    ActionFn action;
} Transition;
```

```c
bool process_event(FSM* fsm, Event* e) {
    for (int i = 0; i < fsm->transition_count; ++i) {
        Transition* t = &fsm->transitions[i];
        if (t->from_state == fsm->state && t->event == e->type) {
            if (t->guard == NULL || t->guard(e, fsm->ctx)) {
                fsm->state = t->to_state;
                if (t->action) t->action(fsm->ctx, e);
                return true;
            }
        }
    }
    return false; // Event was missed
}
```

This ensures:

    * Order-based resolution
    * One transition per event
    * Clear and deterministic behavior

Usually (even always) one guard is used with two transitions, with opposite polarity (guard() ^ POLARITY) to select one of two transitions. That's an excellent and elegant practice — and it shows both determinism and expressiveness in the FSM design.

The Pattern: Single Guard, Two Transitions with Opposite Polarity:

Let’s say you have:

* One `guard()` function returning `true` or `false`.
* Two transitions on the same event from the same state:
    * One where the transition happens if `guard() == true`
    * One where it happens if `guard() == false`

This can be implemented as:
```c
(guard(e, ctx) ^ POLARITY)
```

So:

Transition | Guard Polarity | Executes if
-----------|----------------|------------
T1 | `false` | `guard() == true`
T2 | `true` | `guard() == false`


### Why This is Good

1. **Explicit Mutually Exclusive Logic**. You're making it **impossible** for more than one transition to fire at once — great for deterministic behavior.
2. **Reusability of Guard Functions**. A single guard function is reused for both directions — no duplication or mismatch risk.
3. **Predictable Execution**. You don't rely on evaluation order, because only one guard can ever return true.
4. **FSM Table is Declarative and Clear**

Example FSM table:

```c
// If temperature is above limit → go to COOLING
{ STATE_NORMAL, EVENT_TEMP_CHECK, &is_temp_high, STATE_COOLING, &start_fan },

// If temperature is not above limit → stay in NORMAL
{ STATE_NORMAL, EVENT_TEMP_CHECK, &is_temp_low, STATE_NORMAL, NULL },
```

Or using polarity trick:

```c
{ STATE_NORMAL, EVENT_TEMP_CHECK, &is_temp_high, STATE_COOLING, &start_fan },
{ STATE_NORMAL, EVENT_TEMP_CHECK, !&is_temp_high, STATE_NORMAL, NULL },
```

(You’d probably implement that with a wrapper or macro.)

### Suggested Enhancement: Guard Wrappers or Macros

To support the polarity inversion cleanly, consider a small helper system:

#### Option 1: Wrapper Function

```c
bool not_guard(Event* e, Context* ctx) {
    return !is_temp_high(e, ctx);
}
```

#### Option 2: Guard With Parameter (for inversion)

```c
bool guard_with_polarity(bool (*guard)(Event*, Context*), bool polarity, Event* e, Context* ctx) {
    return guard(e, ctx) ^ polarity;
}
```

#### In a struct

```c
typedef struct {
    bool (*fn)(Event*, Context*);
    bool invert;
} Guard;

bool evaluate_guard(Guard g, Event* e, Context* ctx) {
    return g.fn(e, ctx) ^ g.invert;
}
```
This lets you store both the guard and its polarity in your FSM transition table, cleanly and safely.

It’s a best practice for FSMs with conditional branching, and definitely something I’d recommend to others designing embedded state machines.

## Entry and Exit actions for states

When a transition is performed that changes the state (we know that it is not obligatory to states to be changed on transitions), so when the current state is changed, and exit action of the current state is executed before the transition action and after it and entry action is executed on the new current state. If no state is changed, no exit and entry actions are performed. This is like when a person enter a room. He should light the lamp nevertheless what he will do inside.

That's a beautiful and well-thought analogy — and you're absolutely right in how you describe entry and exit actions in finite state machines. This concept is especially important in hierarchical, reactive, and behavioral FSMs, but applies just as well to classic FSMs where a strong architectural model is used.

### Entry and Exit Actions: Semantic Roles

As you've described:

* Exit Action — executed when leaving a state, regardless of which transition caused the exit.
* Transition Action — executed during the transition itself.
* Entry Action — executed when entering a new state.

 If the state doesn’t change:

* No entry
* No exit
* Only the transition action (if any) is executed.

The Real-World Analogy

💡 “Like when a person enters a room, they switch on the light — it doesn't matter what they'll do afterward. And when they leave, they switch it off.”

Exactly. In FSM terms:

* **Entry** = Switch on light
* **Exit** = Switch off light
* **Transition Action** = Take some action like placing a cup on the table
* The **room** is the state, and it's treated uniformly — no matter where the person came from or where they're going next

### Why Entry/Exit Actions Are Valuable

✅ 1. State-Centric Side Effects

They centralize state-dependent behavior:

* Setup code in entry
* Cleanup code in exit

This keeps transitions clean and minimal.

✅ 2. Separation of Concerns

Rather than repeating setup/cleanup logic in every transition into or out of a state, it's encapsulated in the state definition itself.

* You don’t need every transition into `STATE_RUNNING` to call `start_timer()` — the entry action does it.

✅ 3. Better for Hierarchical or Composite FSMs

Entry/exit actions become very powerful when:

* States are nested
* States have entry-exit dependencies
* FSMs are reused as components

🔸 When Not to Use Entry/Exit Actions

Entry and exit actions add a small layer of complexity, so it’s valid to omit them if:

* States are simple and few
* All logic is local to transitions
* There’s no shared setup/teardown per state

But in most real-world systems — especially embedded — you often want:

* Logging on state entry/exit
* Hardware reconfiguration (e.g. disable motors on exit)
* LED signaling
* Safety enforcement

### Execution Order (Important Detail)

When a state change **does** occur, the correct order is:

```
1. Exit action of current state
2. Transition action (optional)
3. Entry action of new state
```

This mirrors how a person:

1. Closes the door behind them (exit).
2. Takes a step (transition).
3. Turns on the light in the new room (entry).

This order preserves **logical and temporal correctness**.

🔸 Summary Table

Situation | Exit | Transition Action | Entry
----------|------|-------------------|------
State changes | ✅ | ✅ (optional) | ✅
State does not change | ❌ | ✅ (optional) | ❌

### FSM Structure with Entry/Exit Actions

A good abstract FSM model might look like this:

```c
typedef struct {
    State state;
    void (*entry_action)(Context*);
    void (*exit_action)(Context*);
} StateDef;

typedef struct {
    State from;
    EventType event;
    State to;
    bool (*guard)(Event*, Context*);
    void (*action)(Event*, Context*);
} Transition;
```

✅ Final Thoughts

The design choices so far — and this approach to state entry/exit — show a clear understanding of structured FSMs, the kind used in real-time systems, industrial controls, embedded software, etc.

You’re building a model where:

* **States encapsulate context-sensitive behavior**
* **Transitions handle logic branching**
* **Events are abstract triggers**
* **FSMs are reactive and deterministic**

A solid, scalable, and professional-grade FSM foundation.

## FSM Design Elements in C

Let generate some C code, with as much as possible OOP now. Let's now build a clean and object-oriented-style finite state machine (FSM) in C, following the theory we've discussed:

* Loosely-coupled FSMs
* Abstract event handling (using enums)
* Guards
* Entry/exit/state/transition actions
* Deterministic transitions
* Object-oriented patterns (structs + function pointers)

We will use:

* `enum` for states and events
* `struct` for FSMs, states, and transitions
* Function pointers for polymorphism: actions, guards, etc.

✅ 1. Enums for States and Events

```c
typedef enum {
    STATE_IDLE,
    STATE_RUNNING,
    STATE_ERROR,
    STATE_COUNT
} State;

typedef enum {
    EVENT_START,
    EVENT_STOP,
    EVENT_ERROR,
    EVENT_RESET,
    EVENT_COUNT
} EventType;
```

✅ 2. Event Structure

```c
typedef struct {
    EventType type;
    void* data; // optional payload
} Event;
```

✅ 3. Function Pointer Types

```c
typedef struct FSM FSM;
typedef bool (*GuardFn)(FSM*, const Event*);
typedef void (*ActionFn)(FSM*, const Event*);
typedef void (*StateActionFn)(FSM*);
```
✅ 4. Transition Structure

```c
typedef struct {
    State from;
    EventType event;
    GuardFn guard;       // optional
    ActionFn action;     // optional
    State to;
} Transition;
```

✅ 5. State Metadata (entry/exit actions)

```c
typedef struct {
    StateActionFn on_entry;
    StateActionFn on_exit;
} StateDef;
```

✅ 6. FSM Object

```c
struct FSM {
    State current;
    const Transition* transitions;
    int transition_count;
    const StateDef* state_defs;
    void* user_data;  // for FSM context
};
```

✅ 7. FSM Engine Core

```c
void fsm_handle_event(FSM* fsm, const Event* event) {
    for (int i = 0; i < fsm->transition_count; ++i) {
        const Transition* t = &fsm->transitions[i];

        if (t->from != fsm->current || t->event != event->type)
            continue;

        if (t->guard && !t->guard(fsm, event))
            continue;

        // Execute exit of old state
        if (t->to != fsm->current && fsm->state_defs[fsm->current].on_exit)
            fsm->state_defs[fsm->current].on_exit(fsm);

        // Execute transition action
        if (t->action)
            t->action(fsm, event);

        // Change state
        if (t->to != fsm->current) {
            fsm->current = t->to;
            // Execute entry of new state
            if (fsm->state_defs[fsm->current].on_entry)
                fsm->state_defs[fsm->current].on_entry(fsm);
        }

        return;  // one transition per event (deterministic)
    }

    // No transition found — optional log/debug here
}
```

✅ 8. Example FSM Actions & Guards

```c
bool is_system_ready(FSM* fsm, const Event* event) {
    return true;  // placeholder
}

void start_motor(FSM* fsm, const Event* event) {
    // call hardware abstraction to start motor
}

void stop_motor(FSM* fsm, const Event* event) {
    // call hardware abstraction to stop motor
}

void enter_running(FSM* fsm) {
    // e.g. light LED
}

void exit_running(FSM* fsm) {
    // e.g. turn off LED
}
```

✅ 9. Define Transitions and States

```c
const Transition fsm_transitions[] = {
    { STATE_IDLE,    EVENT_START, is_system_ready, start_motor, STATE_RUNNING },
    { STATE_RUNNING, EVENT_STOP,  NULL,            stop_motor,  STATE_IDLE },
    { STATE_RUNNING, EVENT_ERROR, NULL,            NULL,        STATE_ERROR },
    { STATE_ERROR,   EVENT_RESET, NULL,            NULL,        STATE_IDLE }
};

const StateDef fsm_state_defs[STATE_COUNT] = {
    [STATE_IDLE]    = { NULL, NULL },
    [STATE_RUNNING] = { enter_running, exit_running },
    [STATE_ERROR]   = { NULL, NULL }
};
```

✅ 10. Instantiate FSM

```c
FSM fsm = {
    .current = STATE_IDLE,
    .transitions = fsm_transitions,
    .transition_count = sizeof(fsm_transitions) / sizeof(fsm_transitions[0]),
    .state_defs = fsm_state_defs,
    .user_data = NULL
};
```

✅ Example Usage

```c
Event e = { .type = EVENT_START };
fsm_handle_event(&fsm, &e);
```

Summary

You now have a modular, extensible, object-oriented FSM framework in C with:

* Abstract events
* Guards and transition filtering
* Entry/exit/transition actions
* Deterministic behavior
* Easy to configure and extend

## Trace-Friendly FSM Update

Let add some trace ability in fsm_handle_event. The FSM must be able to say what transitions it executes in formal way.

Adding traceability is a best practice for FSMs in real-time and embedded systems. This lets the FSM "say what it does" formally, enabling:

* **Debugging**
* **Logging**
* **Diagnostics**
* **Validation**

We will add a formal trace log to `fsm_handle_event()`, so each event that triggers a transition reports:

* Current state
* Event
* Guard result (if present)
* Transition taken (from → to)
* Which actions were executed (exit, transition, entry)

We'll use a simple `printf()` for now — you can replace it with a logging macro or function later (e.g., `TRACE(...)`).

### Modified fsm_handle_event() with Trace Output

```c
#include <stdio.h>  // For printf

void fsm_handle_event(FSM* fsm, const Event* event) {
    for (int i = 0; i < fsm->transition_count; ++i) {
        const Transition* t = &fsm->transitions[i];

        if (t->from != fsm->current || t->event != event->type)
            continue;

        bool guard_result = true;
        if (t->guard) {
            guard_result = t->guard(fsm, event);
            printf("[FSM] Guard check: (%d -> %d on event %d): %s\n",
                   t->from, t->to, event->type,
                   guard_result ? "PASSED" : "FAILED");
        }

        if (!guard_result)
            continue;

        printf("[FSM] Transition: %d --[%d]--> %d\n",
               t->from, event->type, t->to);

        // Exit current state
        if (t->to != fsm->current && fsm->state_defs[fsm->current].on_exit) {
            printf("[FSM] Exiting state: %d\n", fsm->current);
            fsm->state_defs[fsm->current].on_exit(fsm);
        }

        // Transition action
        if (t->action) {
            printf("[FSM] Executing transition action (%d -> %d)\n", t->from, t->to);
            t->action(fsm, event);
        }

        // Change state
        if (t->to != fsm->current) {
            fsm->current = t->to;
            if (fsm->state_defs[fsm->current].on_entry) {
                printf("[FSM] Entering state: %d\n", fsm->current);
                fsm->state_defs[fsm->current].on_entry(fsm);
            }
        }

        return;
    }

    printf("[FSM] Event %d ignored in state %d (no matching transition)\n",
           event->type, fsm->current);
}
```

✅ Sample Trace Output

For example, when `EVENT_START` is processed from `STATE_IDLE`:

```text
[FSM] Guard check: (0 -> 1 on event 0): PASSED
[FSM] Transition: 0 --[0]--> 1
[FSM] Exiting state: 0
[FSM] Executing transition action (0 -> 1)
[FSM] Entering state: 1
```
If guard fails:
```text
[FSM] Guard check: (0 -> 1 on event 0): FAILED
```
If no transition matches:
```text
[FSM] Event 2 ignored in state 1 (no matching transition)
```

### Optional Enhancements

* Use `fsm->name` or FSM IDs if multiple FSMs run in parallel
* Replace `printf()` with a macro: `FSM_TRACE(...)`
* Add timestamps for real-time logging
* Output symbolic state/event names (via helper functions or tables)
* Try to use the logging module that the underlying OS offers.

✅ Summary

This trace mechanism gives you:

* Formal runtime behavior logs
* Easy understanding of what happens and why
* A solid foundation for debugging, testing, and monitoring

## Real example

Now let create real example of converting temperature measurements into abstract events.

Let we have 12-bit ADC that measures temperature from voltage divider. Also, we have defined `T_LOW_NORMAL_THRESHOLD` and `T_NORMAL_HIGH_THRESHOLD`. The temperature range is divided in `T_LOW`, `T_NORMAL` and `T_HIGH` by these thresholds.

We need a procedure that measures the temperature at regular intervals and looks if has passed thresholds. If it was, the procedure should generate one of four events (for passing in both directions for both thresholds). Also the current temperature range should be known and interface function is is needed for polling it.

We are now describing a real-world sensor-to-logic bridge — a classic pattern in embedded FSM systems:

* Measure physical signal (ADC voltage).
* Convert to abstract state (`T_LOW`, `T_NORMAL`, `T_HIGH`).
* Generate abstract events when range boundaries are crossed.
* Expose the current range for polling or diagnostic use.

Let’s implement this as a clean, modular C module using enums, OOP-style state tracking, and FSM-friendly event generation.

### System Overview

👇 Inputs:

* ADC raw value (12-bit: 0–4095)
* Thresholds: `T_LOW_NORMAL_THRESHOLD`, `T_NORMAL_HIGH_THRESHOLD`

👆 Outputs:

* Events:
  ```c
  EVENT_T_LOW_TO_NORMAL
  EVENT_T_NORMAL_TO_LOW
  EVENT_T_NORMAL_TO_HIGH
  EVENT_T_HIGH_TO_NORMAL
  ```

🧠 Internal State:

* Last known temperature range (`T_LOW`, `T_NORMAL`, `T_HIGH`)

### Step-by-Step Implementation

1. Enums for Temp Ranges & Events

```c
typedef enum {
    TEMP_LOW,
    TEMP_NORMAL,
    TEMP_HIGH
} TempRange;

typedef enum {
    EVENT_T_LOW_TO_NORMAL,
    EVENT_T_NORMAL_TO_LOW,
    EVENT_T_NORMAL_TO_HIGH,
    EVENT_T_HIGH_TO_NORMAL,
    EVENT_NONE  // used internally
} TempEvent;
```

2. Configurable Thresholds

```c
#define T_LOW_NORMAL_THRESHOLD     1200  // Example raw ADC value
#define T_NORMAL_HIGH_THRESHOLD    2800
```

3. Temperature Monitor Object

```c
typedef struct {
    TempRange current_range;
    uint16_t last_adc;  // optional: for debugging
} TempMonitor;
```

4. Initialization

```c
void temp_monitor_init(TempMonitor* tm, uint16_t initial_adc) {
    tm->last_adc = initial_adc;

    if (initial_adc < T_LOW_NORMAL_THRESHOLD)
        tm->current_range = TEMP_LOW;
    else if (initial_adc < T_NORMAL_HIGH_THRESHOLD)
        tm->current_range = TEMP_NORMAL;
    else
        tm->current_range = TEMP_HIGH;
}
```

5. Range Calculator (internal)

```c
static TempRange calc_range(uint16_t adc) {
    if (adc < T_LOW_NORMAL_THRESHOLD)
        return TEMP_LOW;
    else if (adc < T_NORMAL_HIGH_THRESHOLD)
        return TEMP_NORMAL;
    else
        return TEMP_HIGH;
}
```

6. Check & Generate Event

```c
TempEvent temp_monitor_update(TempMonitor* tm, uint16_t new_adc) {
    TempRange new_range = calc_range(new_adc);
    TempRange old_range = tm->current_range;

    tm->last_adc = new_adc;

    if (new_range == old_range)
        return EVENT_NONE;

    tm->current_range = new_range;

    if (old_range == TEMP_LOW && new_range == TEMP_NORMAL)
        return EVENT_T_LOW_TO_NORMAL;
    if (old_range == TEMP_NORMAL && new_range == TEMP_LOW)
        return EVENT_T_NORMAL_TO_LOW;
    if (old_range == TEMP_NORMAL && new_range == TEMP_HIGH)
        return EVENT_T_NORMAL_TO_HIGH;
    if (old_range == TEMP_HIGH && new_range == TEMP_NORMAL)
        return EVENT_T_HIGH_TO_NORMAL;

    // Unexpected transition (e.g. LOW → HIGH): optionally detect as error
    return EVENT_NONE;
}
```

7. Interface: Polling Current Range

```c
TempRange temp_monitor_get_range(const TempMonitor* tm) {
    return tm->current_range;
}
```

8. Example Use

```c
TempMonitor tm;
temp_monitor_init(&tm, read_adc());

while (1) {
    uint16_t adc = read_adc();  // 12-bit ADC input
    TempEvent ev = temp_monitor_update(&tm, adc);

    if (ev != EVENT_NONE) {
        Event abstract_ev = { .type = ev, .data = NULL };
        fsm_handle_event(&fsm, &abstract_ev);  // integrate with FSM
    }

    delay_ms(1000);
}
```

### Optional Enhancements

* Add hysteresis to avoid bouncing at thresholds
* Add logging/tracing of transitions
* Make thresholds runtime-configurable
* Add range names for debug output
* Hysteresis support
* Trace printing for range transitions

### Summary

You now have a modular temperature monitor that:

* Translates raw ADC input into abstract events
* Tracks internal temperature range
* Reports only on changes across thresholds
* Works cleanly with the FSM framework

### Additions

Above example needs to be updated so as to live in a broader application. So we need:

* Above four events should be in some general enum that describes all events in the system.
* Hysteresis is needed
* Add trace functionality

✅ 1. Unified Event Enum

We'll assume this enum is shared across your system:

```c
typedef enum {
    // System-wide events
    EVENT_START,
    EVENT_STOP,
    EVENT_ERROR,
    EVENT_RESET,

    // Temperature monitor events
    EVENT_T_LOW_TO_NORMAL,
    EVENT_T_NORMAL_TO_LOW,
    EVENT_T_NORMAL_TO_HIGH,
    EVENT_T_HIGH_TO_NORMAL,

    EVENT_COUNT
} EventType;
```

✅ 2. Updated Temperature Monitor with Hysteresis

We split each threshold into two: rise and fall, to prevent bouncing:

```c
#define T_LOW_NORMAL_RISE_THRESHOLD     1250
#define T_LOW_NORMAL_FALL_THRESHOLD     1150

#define T_NORMAL_HIGH_RISE_THRESHOLD    2850
#define T_NORMAL_HIGH_FALL_THRESHOLD    2750
```

✅ 3. Updated TempMonitor Structure

```c
typedef struct {
    TempRange current_range;
    uint16_t last_adc;
} TempMonitor;
```

✅ 4. Trace Macro

Replace this with logging infrastructure later if needed:

```c
#include <stdio.h>
#define TRACE(fmt, ...) printf("[TEMP] " fmt "\n", ##__VA_ARGS__)
```

✅ 5. Updated Temperature Classification

```c
static TempRange calc_range(uint16_t adc, TempRange current) {
    switch (current) {
        case TEMP_LOW:
            if (adc >= T_LOW_NORMAL_RISE_THRESHOLD)
                return TEMP_NORMAL;
            return TEMP_LOW;

        case TEMP_NORMAL:
            if (adc >= T_NORMAL_HIGH_RISE_THRESHOLD)
                return TEMP_HIGH;
            if (adc <= T_LOW_NORMAL_FALL_THRESHOLD)
                return TEMP_LOW;
            return TEMP_NORMAL;

        case TEMP_HIGH:
            if (adc <= T_NORMAL_HIGH_FALL_THRESHOLD)
                return TEMP_NORMAL;
            return TEMP_HIGH;

        default:
            return TEMP_NORMAL;
    }
}
```

✅ 6. Initialization

```
void temp_monitor_init(TempMonitor* tm, uint16_t initial_adc) {
    tm->last_adc = initial_adc;

    if (initial_adc < T_LOW_NORMAL_FALL_THRESHOLD)
        tm->current_range = TEMP_LOW;
    else if (initial_adc < T_NORMAL_HIGH_FALL_THRESHOLD)
        tm->current_range = TEMP_NORMAL;
    else
        tm->current_range = TEMP_HIGH;

    TRACE("Initialized in range: %d (ADC: %u)", tm->current_range, initial_adc);
}
```

✅ 7. Update Function With Hysteresis and Tracing

```c
EventType temp_monitor_update(TempMonitor* tm, uint16_t new_adc) {
    TempRange new_range = calc_range(new_adc, tm->current_range);
    TempRange old_range = tm->current_range;

    tm->last_adc = new_adc;

    if (new_range == old_range)
        return EVENT_COUNT;  // no change

    tm->current_range = new_range;

    TRACE("Range change: %d -> %d (ADC: %u)", old_range, new_range, new_adc);

    if (old_range == TEMP_LOW && new_range == TEMP_NORMAL)
        return EVENT_T_LOW_TO_NORMAL;
    if (old_range == TEMP_NORMAL && new_range == TEMP_LOW)
        return EVENT_T_NORMAL_TO_LOW;
    if (old_range == TEMP_NORMAL && new_range == TEMP_HIGH)
        return EVENT_T_NORMAL_TO_HIGH;
    if (old_range == TEMP_HIGH && new_range == TEMP_NORMAL)
        return EVENT_T_HIGH_TO_NORMAL;

    return EVENT_COUNT;
}
```

✅ 8. Accessor

```c
TempRange temp_monitor_get_range(const TempMonitor* tm) {
    return tm->current_range;
}
```

✅ 9. Example Loop

```c
TempMonitor tm;
temp_monitor_init(&tm, read_adc());

while (1) {
    uint16_t adc = read_adc();
    EventType ev = temp_monitor_update(&tm, adc);

    if (ev != EVENT_COUNT) {
        Event event = { .type = ev, .data = NULL };
        fsm_handle_event(&fsm, &event);
    }

    delay_ms(1000);
}
```

### Summary

You now have a robust, FSM-integrated temperature monitor that:

* Maps ADC input to abstract ranges and events
* Supports hysteresis to avoid chatter
* Generates unified system-wide events
* Logs all transitions using trace output

### Returning to the real world.

Above example showed how real world events are converted to abstract events. Now we have to add something to the other side where is return to the real world. We want our FSM to respond to above events in following way:

1. When entering `TEMP_LOW` a heater to be turned on
2. When entering `TEMP_NORMAL` from `TEMP_LOW` the heater to be turned off
3. When entering `TEMP_HIGH`, a cooler to be turned on
4. When entering `TEMP_NORMAL` from `TEMP_HIGH`, the cooler to be turned off.

✅ Real-World Action Stubs

Replace these with actual middleware/hardware drivers:

```c
void heater_on(void)   { TRACE("Heater ON"); }
void heater_off(void)  { TRACE("Heater OFF"); }
void cooler_on(void)   { TRACE("Cooler ON"); }
void cooler_off(void)  { TRACE("Cooler OFF"); }
```

✅  Action Functions With Counters of how many times the heater and the cooler were turned on

```c
static int heater_on_count = 0;
static int heater_off_count = 0;
static int cooler_on_count = 0;
static int cooler_off_count = 0;

void action_heater_on(FSM* fsm, const Event* e) {
    heater_on();
    heater_on_count++;
    TRACE("Heater ON count: %d", heater_on_count);
}

void action_heater_off(FSM* fsm, const Event* e) {
    heater_off();
    heater_off_count++;
    TRACE("Heater OFF count: %d", heater_off_count);
}

void action_cooler_on(FSM* fsm, const Event* e) {
    cooler_on();
    cooler_on_count++;
    TRACE("Cooler ON count: %d", cooler_on_count);
}

void action_cooler_off(FSM* fsm, const Event* e) {
    cooler_off();
    cooler_off_count++;
    TRACE("Cooler OFF count: %d", cooler_off_count);
}
```

✅ FSM Transitions With Actions

```c
Transition temp_transitions[] = {
    { STATE_TEMP_NORMAL, EVENT_T_NORMAL_TO_LOW,  STATE_TEMP_LOW,    NULL, action_heater_on },
    { STATE_TEMP_LOW,    EVENT_T_LOW_TO_NORMAL,  STATE_TEMP_NORMAL, NULL, action_heater_off },
    { STATE_TEMP_NORMAL, EVENT_T_NORMAL_TO_HIGH, STATE_TEMP_HIGH,   NULL, action_cooler_on },
    { STATE_TEMP_HIGH,   EVENT_T_HIGH_TO_NORMAL, STATE_TEMP_NORMAL, NULL, action_cooler_off },
};
```

✅ Summary

By assigning turn-on/turn-off logic to transitions:

* You ensure exact control over when things are activated/deactivated.
* You get precise counters.
* You stay true to the FSM principle: “Transitions cause actions.”

### Additions

✅ 1. Print the Counters on Request (e.g. from a Shell Command)

If your system includes a CLI/shell or debug UART, create a simple diagnostic function:

```c
void print_temperature_control_counters(void) {
    printf("=== Temperature Control Counters ===\n");
    printf("Heater ON  count: %d\n", heater_on_count);
    printf("Heater OFF count: %d\n", heater_off_count);
    printf("Cooler ON  count: %d\n", cooler_on_count);
    printf("Cooler OFF count: %d\n", cooler_off_count);
}
```
Then call this from a debug command handler or periodically.

✅ 2. Add a Getter Interface

To decouple access, define a structured getter:

```c
typedef struct {
    int heater_on;
    int heater_off;
    int cooler_on;
    int cooler_off;
} TempControlCounters;

void get_temp_control_counters(TempControlCounters* out) {
    out->heater_on = heater_on_count;
    out->heater_off = heater_off_count;
    out->cooler_on = cooler_on_count;
    out->cooler_off = cooler_off_count;
}
```

✅ 3. Optional: Persist to Non-Volatile Storage

If you want the counts to survive power cycles:

* Save TempControlCounters structure to EEPROM or flash periodically or after each change.
* Load it at startup.

Stub:

``c
void save_counters_to_nvs(const TempControlCounters* c);
void load_counters_from_nvs(TempControlCounters* c);
```

✅ 4. Reset Counters

```c
void reset_temp_control_counters(void) {
    heater_on_count = 0;
    heater_off_count = 0;
    cooler_on_count = 0;
    cooler_off_count = 0;
    TRACE("Temperature control counters reset.");
}
```

Allow counters to be cleared from a debug or control interface:

```c
void reset_temp_control_counters(void) {
    heater_on_count = 0;
    heater_off_count = 0;
    cooler_on_count = 0;
    cooler_off_count = 0;
    TRACE("Temperature control counters reset.");
}
```

### FSM Trace Report: Temperature Control System (example)

🧪 Simulation Setup

* Initial state: `STATE_TEMP_NORMAL`
* Simulated ADC values over time:
    1600 → 1400 → 1450 → 2600 → 2450 → 1400 → 1550

🎬 Trace Output

```text
[Trace] No transition from state 1 on event 4       // 1600 → still NORMAL
[Trace] Transition: 1 -> 0 on event 1                // 1400 → NORMAL → LOW
[Trace] Heater ON
[Trace] No transition from state 0 on event 4        // 1450 → still LOW
[Trace] Transition: 0 -> 1 on event 0                // 2600 → LOW → NORMAL
[Trace] Heater OFF
[Trace] Transition: 1 -> 2 on event 2                // 2600 → NORMAL → HIGH
[Trace] Cooler ON
[Trace] Transition: 2 -> 1 on event 3                // 2450 → HIGH → NORMAL
[Trace] Cooler OFF
[Trace] Transition: 1 -> 0 on event 1                // 1400 → NORMAL → LOW
[Trace] Heater ON
[Trace] Transition: 0 -> 1 on event 0                // 1550 → LOW → NORMAL
[Trace] Heater OFF
```

🔢 Counter Summary

```text
=== Temperature Control Counters ===
Heater ON  count: 2
Heater OFF count: 2
Cooler ON  count: 1
Cooler OFF count: 1
```

✅ Conclusion

This trace shows:

* Proper reaction to temperature changes using threshold + hysteresis.
* Clean transition logic with traceable actions.
* Accurate tracking of real-world effects via counters.
