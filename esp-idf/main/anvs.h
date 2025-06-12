// anvs_api.h

#if !defined(ANVS_API_H)
#define ANVS_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <esp_err.h>

#include "nvs_flash.h"

#define APP_STORAGE         "appstore"
#define APP_STORAGE_MARK    "appmark"

esp_err_t anvs_initialize(void);
void anvs_stop_nvs_commit_task(void);
esp_err_t anvs_check_appstore(void);
esp_err_t anvs_init_appstore(void);
esp_err_t anvs_dump_appstore(void);

esp_err_t anvs_app_op_mode_get(uint16_t* value);
esp_err_t anvs_app_op_mode_set(uint16_t value);

esp_err_t anvs_u16_get(const char* key, uint16_t* value);
esp_err_t anvs_u16_set(const char* key, uint16_t value);

esp_err_t read_opmode(void);

#ifdef __cplusplus
}
#endif

#endif  // !defined(ANVS_API_H)

// End of anvs_api.h