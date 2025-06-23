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
#include "esp_timer.h"

#include "commondefs.h"
#include "anvs.h"
#include "sm.h"

static const char TAG[] = "proc";

typedef struct {
    device_modes_t opmode;  // actual operative mode

    portMUX_TYPE mux;
} operative_state_t;

static operative_state_t operative_state = {
    .opmode = OP_MODE_STANDBY,
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

// INPUT DEVICE

// button handling


button_handle_t btn;

static void button_event_cb(void *arg, void *data)
{
    button_event_t event = iot_button_get_event(arg);
    ESP_LOGI(TAG, "%s", iot_button_get_event_str(event));
    sm_post_event(evButtonSingleClick);
}

void init_button(void)
{
    button_config_t btn_cfg = {0};

    button_gpio_config_t gpio_cfg = {
        .gpio_num = CONFIG_BUTTON_GPIO,
        .active_level = CONFIG_BUTTON_ACTIVE_LEVEL,
        .enable_power_save = false,
    };

    esp_err_t ret = iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &btn);

    iot_button_register_cb(btn, BUTTON_SINGLE_CLICK, NULL, button_event_cb, NULL);
}

// OUTPUT DEVICE

// led handling

// Blinking intervals (full period in ms)
static const int blink_intervals[] = {100, 500, 1000, 2000, 2500};  // 10Hz to 0.4Hz

static volatile int current_blink_index = 0;
static esp_timer_handle_t led_timer = NULL;
static bool led_state = !CONFIG_LED_ACTIVE_LEVEL;

// LED toggle callback (called from esp_timer)
static void led_timer_callback(void* arg)
{
    led_state = !led_state;
    gpio_set_level(CONFIG_LED_GPIO, led_state);
}

void set_blink_period(int index)
{
    if (index < 0 || index >= ARRAY_SIZE(blink_intervals)) {
        ESP_LOGE(TAG, "Invalid blink index: %d", index);
        return;
    }

    current_blink_index = index;

    if (led_timer) {
        esp_timer_stop(led_timer);
        esp_timer_start_periodic(led_timer, (blink_intervals[current_blink_index] / 2) * 1000);
    }
}

void init_led_blinking(void)
{
    gpio_reset_pin(CONFIG_LED_GPIO);
    gpio_set_direction(CONFIG_LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(CONFIG_LED_GPIO, !CONFIG_LED_ACTIVE_LEVEL);  // Start with LED off

    esp_timer_create_args_t timer_args = {
        .callback = &led_timer_callback,
        .name = "led_blink_timer",
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .skip_unhandled_events = false,
    };

    esp_err_t ret = esp_timer_create(&timer_args, &led_timer);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create LED timer: %s", esp_err_to_name(ret));
        return;
    }

    set_blink_period(0);  // Start with the first blink interval
}
