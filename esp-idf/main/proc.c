// proc.c

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "anvs.h"
#include "commondefs.h"

typedef struct {
    device_modes_t opmode;  // actual operative mode

    portMUX_TYPE mux;
} operative_state_t;

static operative_state_t operative_state = {
    .opmode = OP_MODE_IDLE,
    .mux = portMUX_INITIALIZER_UNLOCKED
 };

void set_opmode(device_modes_t mode)
{
    portENTER_CRITICAL(&operative_state.mux);
    operative_state.opmode = mode;
    portEXIT_CRITICAL(&operative_state.mux);
}

device_modes_t get_opmode(void)
{
    device_modes_t mode;
    portENTER_CRITICAL(&operative_state.mux);
    mode = operative_state.opmode;
    portEXIT_CRITICAL(&operative_state.mux);
    return mode;
}

esp_err_t read_opmode(void)
{
    return anvs_app_op_mode_get((uint16_t* )&operative_state.opmode);
}
