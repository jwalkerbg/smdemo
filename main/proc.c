// proc.c
#include "sdkconfig.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "iot_button.h"
#include "button_gpio.h"

#include "commondefs.h"
#include "anvs.h"

// static const char TAG[] = "proc";

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
    portENTER_CRITICAL(&operative_state.mux);
    esp_err_t ret = anvs_app_op_mode_get((uint16_t* )&operative_state.opmode);
    portEXIT_CRITICAL(&operative_state.mux);
    return ret;
}

// button handling

#define BUTTON_GPIO 12
#define BUTTON_ACTIVE_LEVEL 0

button_config_t btn_cfg = {0};

button_gpio_config_t gpio_cfg = {
    .gpio_num = BUTTON_GPIO,
    .active_level = BUTTON_ACTIVE_LEVEL,
    .enable_power_save = false,
};

button_handle_t btn;


void init_button(void)
{
    esp_err_t ret = iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &btn);

    iot_button_register_cb(btn, BUTTON_SINGLE_CLICK, NULL, button_event_cb, NULL);
}
