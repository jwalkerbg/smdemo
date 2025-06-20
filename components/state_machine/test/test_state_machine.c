// test_led_api.c: Implementation of a testable component.

#include <limits.h>
#include <inttypes.h>

#include "commondefs.h"

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "unity.h"
#include "sm.h"

TEST_CASE("sm_init","[sm]")
{
	TEST_ASSERT_TRUE(true);
}
