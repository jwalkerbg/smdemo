// commondefs.h

#pragma once

#include "sdkconfig.h"

#if defined(__cplusplus)
extern "C" {    // allow use with C++ compilers
#endif

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]) + \
                         __builtin_types_compatible_p(typeof(arr), typeof(&(arr)[0])) * -1)

#define ARRAY_SIZE_2D(arr) (sizeof(arr) / sizeof((arr)[0]) + \
                         __builtin_types_compatible_p(typeof(arr), typeof(&(arr)[0])) * -1 + \
                         __builtin_types_compatible_p(typeof(arr[0]), typeof(&(arr[0])[0])) * -1)

#define SERIAL_LENGTH   (32)

#define WHO_CODE    (0x5A)

typedef enum {
    OP_MODE_IDLE = 0,
    OP_MODE_AUTO,
    OP_MODE_NIGHT,
    OP_MODE_MANUAL,
    OP_MODE_TEST,

    OP_MODE_COUNT
} device_modes_t;

#if defined(__cplusplus)
}   // end of extern "C"
#endif

// enf of commondefs.h
