idf_component_register(SRCS "sm.c"
                    INCLUDE_DIRS "."
                    REQUIRES main driver esp_event
)

# Pass the version to the build system
set(SM_VERSION "${CONFIG_SM_VERSION}")

message(STATUS "SM_VERSION: ${SM_VERSION}")