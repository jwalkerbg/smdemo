// process.h

#pragma once

#if defined(__cplusplus)
extern "C" {    // allow use with C++ compilers
#endif

#include "sdkconfig.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "sm.h"

// sm_P1 Main process ================================================

#define P1_ID   (1)
#define P1_STATES \
    X(sP1_START) X(sP1_RESOLVE) \
    X(sP1_STANDBY) X(sP1_AUTO) X(sP1_AUTO_NIGHT) X(sP1_MANUAL) X(sP1_TEST)

typedef enum sP1_states {
    #define X(name) name,
    P1_STATES
    #undef X
    sP1_STATE_COUNT
} sP1_states_t;

typedef struct {
    uint16_t dummy;
} P1_context_t;

void P1_start(void);
void P1_stop(void);

esp_err_t register_state_machines(void);

#if defined(__cplusplus)
}   // end of extern "C"
#endif

// end of process.h
