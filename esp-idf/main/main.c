// main.c

#include <stdio.h>
#include <stdlib.h>
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "sm.h"
#include "process.h"
#include "anvs.h"

static char TAG[] = "APP";

// Application main
void app_main(void)
{
    esp_err_t ret;

    ESP_LOGI(TAG,"Application version: %s",CONFIG_APP_PROJECT_VER);

    /* Initialize NVS. */
    ret = anvs_initialize();
    if (ret == ESP_OK) {
        ret = anvs_check_appstore();
        if (ret == ESP_ERR_NVS_NOT_FOUND) {
            ret = anvs_init_appstore();
        }
    }

    if ((ret = register_state_machines()) != ESP_OK) {
        ESP_LOGI(TAG,"Not all state machines are registered : %d. This is implementation error",ret);
    }
    sm_create_event_loop();

    P1_start();
}