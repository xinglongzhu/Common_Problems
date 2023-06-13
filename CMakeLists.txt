# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_LIST_DIR})

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/boards/app.overlay")
    set(DTC_OVERLAY_FILE "${CMAKE_CURRENT_LIST_DIR}/boards/app.overlay")
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(distance_test)

FILE(GLOB app_sources cpuapp/src/*.c)

target_sources(app 
  PRIVATE
  ${app_sources}
  cpuapp/main.c
)

target_include_directories(app
  PRIVATE
  cpuapp/include
)
 