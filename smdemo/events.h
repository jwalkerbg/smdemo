#ifndef EVENTS_H
#define EVENTS_H

enum events_list {
    evNullEvent,

// TO DO: insert application events here
    evBTN1Pressed, evBTN1Released,
    evBTN2Pressed, evBTN2Released,
    evBTN3Pressed, evBTN3Released,
    evP1Trigger1, evP1Trigger2, evP1Trigger3, evP1Trigger4,

    evP1Trigger5, // changing nothing event
#if defined(TEST)
    evFakeEvent,
#endif  // defined(TEST)

    evEventsNumber
};
typedef enum events_list EVENT_TYPE;

#endif // EVENTS_H
