cmake_minimum_required(VERSION 3.5)

project(smdemo LANGUAGES C)

add_definitions(-DSM_TRACER)

include_directories(./)

file(GLOB MY_SOURCES *.c)
file(GLOB MY_HEADERS *.h)

add_executable(smdemo ${MY_SOURCES} ${MY_HEADERS})

install(TARGETS smdemo
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
