// proc.h

#pragma once

#if defined(__cplusplus)
extern "C" {    // allow use with C++ compilers
#endif

#include <stdint.h>
#include <stdbool.h>
#include <esp_err.h>

void set_opmode(device_modes_t mode);
device_modes_t get_opmode(void);
esp_err_t read_opmode(void);

void init_button(void);
void init_led_blinking(void);
void set_blink_period(int index);

#if defined(__cplusplus)
}   // end of extern "C"
#endif

// end of util.h