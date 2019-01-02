cmake_minimum_required(VERSION 3.8)
project(gba_bt_hid)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})
include(gba_rules)

set(CMAKE_SYSTEM_NAME Generic)
#set(CMAKE_SYSTEM_PROCESSOR armv4)
#set(CMAKE_CXX_STANDARD 11)

set(DEVKITARM $ENV{DEVKITARM})
set(DEVKITPRO $ENV{DEVKITPRO})

if(NOT DEVKITPRO)
    message(FATAL_ERROR "DEVKITPRO environment variable not set")
endif()

if(NOT DEVKITARM)
    message(FATAL_ERROR "DEVKITARM environment variable not set")
endif()

set(CMAKE_C_COMPILER ${DEVKITARM}/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER ${DEVKITARM}/bin/arm-none-eabi-g++)

set(CMAKE_FIND_ROOT_PATH ${DEVKITARM})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_FIND_LIBRARY_PREFIXES lib)
set(CMAKE_FIND_LIBRARY_SUFFIXES .a)

include_directories(${DEVKITPRO}/libgba/include)
link_directories(${DEVKITPRO}/libgba/lib)
find_library(GBA gba)

add_definitions(-DARM7)

set(EXE_NAME gba_bt_hid_mb)
file(GLOB_RECURSE SOURCE_FILES
    source/*
)
add_executable(${EXE_NAME} ${SOURCE_FILES})

target_link_libraries(${EXE_NAME} GBA)
set_target_properties(${EXE_NAME}
        PROPERTIES
        LINK_FLAGS -specs=gba_mb.specs
        COMPILER_FLAGS "-mthumb -mthumb-interwork")

objcopy_file(${EXE_NAME})
gbafix_file(${EXE_NAME})
