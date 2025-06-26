idf_component_register(SRCS
        "main.c"
        "process.c"
        "anvs.c"
        "proc.c"
        INCLUDE_DIRS "." "include"
        REQUIRES esp_timer nvs_flash
)

# Pass the version to the build system
set(APP_PROJECT_VER "${CONFIG_APP_PROJECT_VER}")

message(STATUS "APP_PROJECT_VER: ${APP_PROJECT_VER}")
