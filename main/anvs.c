// anvs.c

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

#include "commondefs.h"

#include "anvs.h"

static const char TAG[] = "ANVS";

#define NVS_CHANGED   BIT0  // Set when NVS is modified
#define NVS_COMMITTED BIT1  // Set when commit is done
#define NVS_EXIT      BIT2  // Set to exit the commit task
#define NVS_CFAILED   BIT3  // Failed to commit

static EventGroupHandle_t nvs_event_group;
static nvs_handle_t app_nvs_handle = 0;

static const char app_storage_marker_key[] = APP_STORAGE_MARK;
static const char app_operative_mode_key[] = "opmode";

static void nvs_commit_task(void *pvParameter);
static esp_err_t anvs_wait_commit(void);

esp_err_t anvs_initialize(void)
{
    esp_err_t ret;

    /* Initialize NVS. */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK_WITHOUT_ABORT(ret);

    if (ret == ESP_OK) {
        nvs_event_group = xEventGroupCreate();
        xTaskCreatePinnedToCore(nvs_commit_task, "NVS_Commit", 4096, NULL, 5, NULL, 1);
    }

    return ret;
}

// static esp_err_t anvs_open_appstore(void)
// Input: none
// Output: none
// Description: This function opens appstore and stores its handle in app_nvs_handle.
static esp_err_t anvs_open_appstore(void)
{
    esp_err_t ret;

    ret = nvs_open(APP_STORAGE,NVS_READWRITE,&app_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG,"Cannot open nvs handle: %s",esp_err_to_name(ret));
    }
    return ret;
}

// static void nvs_commit_task(void *pvParameter)
// Input: none
// Output: none
// Description: This function is executed as a task by CORE1. It waits for commands (event bits)
// NVS_CHANGED: to execute nvs_commit()
// NVS_EXIT: to exit.
// When nvs_commit is requested, it is executed and then NVS_COMMITTED is isgnaled. This allows the functions
// that requested commit to know that it was executed successfully. If the commit is not successful, then
// NVS_CFAILED is set.

static void nvs_commit_task(void *pvParameter)
{

    ESP_LOGI(TAG, "nvs_commit_task entered");
    while (true) {
        // Wait for NVS_CHANGED flag
        EventBits_t bits = xEventGroupWaitBits(nvs_event_group, NVS_CHANGED, pdTRUE, pdFALSE, portMAX_DELAY);
        // If NVS_CHANGED is received, commit changes
        if (bits & NVS_CHANGED) {
            // Commit changes to flash
            if (nvs_commit(app_nvs_handle) == ESP_OK) {
                ESP_LOGI(TAG,"NVS data committed successfully.");
                // Signal that commit is done
                xEventGroupSetBits(nvs_event_group, NVS_COMMITTED);
            }
            else {
                // Signal that commit failed
                xEventGroupSetBits(nvs_event_group, NVS_CFAILED);
            }
        }

        if (bits & NVS_EXIT) {
            break;
        }
    }
    ESP_LOGI(TAG, "nvs_commit_task exited");
    vTaskDelete(NULL);
}

static esp_err_t anvs_wait_commit(void)
{
    xEventGroupSetBits(nvs_event_group, NVS_CHANGED);
    EventBits_t bits = xEventGroupWaitBits(nvs_event_group, NVS_COMMITTED | NVS_CFAILED, pdTRUE, pdFALSE, portMAX_DELAY);
    if (bits & NVS_COMMITTED) {
        return ESP_OK;
    }
    else {
        return ESP_FAIL;
    }
}

void anvs_stop_nvs_commit_task(void)
{
    ESP_LOGI(TAG,"Stopping NVS commit task...");
    xEventGroupSetBits(nvs_event_group, NVS_EXIT);  // Signal the task to exit
}

// esp_err_t anvs_check_appstore(void)
// Input: none
// Output: ESP error code
// Description: This function checks the existence of appstore. It tries to read the value
//  with key APP_STORAGE_MARK. If this value exists appstore exists, otherwise it does not exists.
esp_err_t anvs_check_appstore(void)
{
    esp_err_t ret;

    ret = anvs_open_appstore();
    if (ret != ESP_OK) {
        return ret;
    }

    // read marker to see if there is app record.
    // The marker is simply an integer; it exists if a record has been written.
    uint16_t marker;
    ret = nvs_get_u16(app_nvs_handle, APP_STORAGE_MARK, &marker);
    switch (ret) {
    case ESP_OK:
        ESP_LOGI(TAG, "appstore exists");
        break;
    case ESP_ERR_NVS_NOT_FOUND:
        ESP_LOGI(TAG, "appstore does not exist");
        break;
    default :
        ESP_LOGI(TAG, "error reading appstore");
    }
    nvs_close(app_nvs_handle);
    return ret;
}

// esp_err_t anvs_init_appstore(void)
// Input: none
// Output: ESP error code
// Descrition: Initializes the appstore with "factory" values.
esp_err_t anvs_init_appstore(void)
{
    esp_err_t ret;

    ret = anvs_open_appstore();
    if (ret != ESP_OK) {
        return ret;
    }

    ESP_LOGI(TAG, "Restoring appstore to factory values");

    // marker
    uint16_t marker = 1;
    nvs_set_u16(app_nvs_handle,app_storage_marker_key,marker);

    // operative mode
    uint16_t op_mode = OP_MODE_STANDBY;
    ret = nvs_set_u16(app_nvs_handle,app_operative_mode_key,op_mode);

    ret = anvs_wait_commit();

    nvs_close(app_nvs_handle);

    return ret;
}

// esp_err_t anvs_dump_appstore(void)
// Input: none
// Output: ESP error code
// Description: This function dump appstore
esp_err_t anvs_dump_appstore(void)
{
    size_t length;
    uint16_t value;
    char sdata[80];

    int ret = anvs_open_appstore();
    if (ret != ESP_OK) {
        return ret;
    }
    nvs_iterator_t it = NULL;
    esp_err_t res = nvs_entry_find("nvs", APP_STORAGE, NVS_TYPE_ANY, &it);
    while(res == ESP_OK) {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info); // Can omit error check if parameters are guaranteed to be non-NULL

        switch (info.type) {
        case NVS_TYPE_STR:
            nvs_get_str(app_nvs_handle,info.key,NULL,&length);
            nvs_get_str(app_nvs_handle,info.key,sdata,&length);
            ESP_LOGI(TAG,"key '%s', type '%d', value '%s'", info.key, info.type,sdata);
            break;
        case NVS_TYPE_U16:
            nvs_get_u16(app_nvs_handle,info.key,&value);
            ESP_LOGI(TAG,"key '%s', type '%d', value '%u'", info.key, info.type,value);
            break;
        default:
            ESP_LOGI(TAG,"key '%s', type '%d'", info.key, info.type);
            break;
        }
        res = nvs_entry_next(&it);
    }
    nvs_release_iterator(it);
    nvs_close(app_nvs_handle);
    return ret;
}

// esp_err_t anvs_app_op_mode_get(uint16_t* value)
// Input:
//  value: pointer to a variable where saved operative mode to be written
// Output:
//  return code from anvs_u16_get()
// Description: This function reads operative mode from anvs. It is used on [re]start of the system
// to restore the mode as it was before the restart.
esp_err_t anvs_app_op_mode_get(uint16_t* value)
{
    return anvs_u16_get(app_operative_mode_key,value);
}

esp_err_t anvs_app_op_mode_set(uint16_t value)
{
    return anvs_u16_set(app_operative_mode_key,value);
}

// esp_err_t anvs_u16_get(const char* key, uint16_t* value)
// Input:
//  key: pointer to key in anvs
//  value: pointer to a variable where the value read to be written
// Description: Ths function opens appstore, reads the value with key 'key' and then closes anvs.
//  The function is a wrapper of nvs_get_u16() and is used for data with uint6_t type.
esp_err_t anvs_u16_get(const char* key, uint16_t* value)
{
    int ret = anvs_open_appstore();
    if (ret != ESP_OK) {
        return ret;
    }
    ret = nvs_get_u16(app_nvs_handle,key,value);
    nvs_close(app_nvs_handle);
    return ret;
}

// esp_err_t anvs_u16_set(const char* key, uint16_t value)
// Input:
//  key: key of the value to be written
//  value: the value to be vriten
// Output: ESP error code
// Description: This function saves key:value in anvs.
esp_err_t anvs_u16_set(const char* key, uint16_t value)
{
    int ret = anvs_open_appstore();
    if (ret != ESP_OK) {
        return ret;
    }
    ret = nvs_set_u16(app_nvs_handle,key,value);

    ret = anvs_wait_commit();

    nvs_close(app_nvs_handle);
    return ret;
}
