###############################################################################
# Host Test Build - GUGU MACHINE GUIDANCE V2
#
# Usage:
#   cd test && cmake -B build && cmake --build build && ctest --test-dir build --output-on-failure
#   Or simply: bash test/run_tests.sh
#
# Test categories:
#   unit/    - PURE modules (no HAL, no RTOS, no mocks needed)
#   module/  - MOCKABLE modules (HAL abstracted, mock FreeRTOS/HAL stubs)
###############################################################################

cmake_minimum_required(VERSION 3.14)
project(gugu_test C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Warnings
add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)

# Unity: enable float/double support explicitly
add_compile_definitions(UNITY_INCLUDE_DOUBLE UNITY_INCLUDE_FLOAT)

# ---- Paths ----
set(ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(MOCK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mock)

# ---- Mock headers FIRST (intercept FreeRTOS.h, stm32f4xx_hal.h, etc.) ----
include_directories(BEFORE SYSTEM ${MOCK_DIR})

# ---- Project include paths ----
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/unity
    ${CMAKE_CURRENT_SOURCE_DIR}/fixture
    ${ROOT}/lib/parser
    ${ROOT}/lib/utils/inc
    ${ROOT}/lib/gps
    ${ROOT}/lib/log
    ${ROOT}/config
)

# ---- Unity library ----
add_library(unity STATIC unity/unity.c)

# ---- Mock/Stub library (shared by module tests) ----
add_library(mock_common STATIC mock/mock_common.c)

add_library(gps_stubs STATIC mock/gps_stubs.c)
target_include_directories(gps_stubs PRIVATE
    ${MOCK_DIR}
    ${ROOT}/lib/gps
    ${ROOT}/lib/utils/inc
    ${ROOT}/lib/log
    ${ROOT}/config
)

# nmea_try_parse stub (for tests that link gps_parser.c but not gps_nmea.c)
add_library(gps_stubs_nmea STATIC mock/gps_stubs_nmea.c)
target_include_directories(gps_stubs_nmea PRIVATE
    ${MOCK_DIR}
    ${ROOT}/lib/gps
    ${ROOT}/lib/utils/inc
    ${ROOT}/lib/log
    ${ROOT}/config
)

# ---- Project source files ----
set(SRC_PARSER      ${ROOT}/lib/parser/parser.c)
set(SRC_RINGBUFFER  ${ROOT}/lib/utils/src/ringbuffer.c)
set(SRC_GPS_NMEA    ${ROOT}/lib/gps/gps_nmea.c)
set(SRC_GPS_PARSER  ${ROOT}/lib/gps/gps_parser.c)

###############################################################################
# Unit Tests (PURE modules - no mock needed)
###############################################################################

# test_parser: lib/parser/parser.c
add_executable(test_parser
    unit/test_parser.c
    ${SRC_PARSER}
)
target_link_libraries(test_parser unity)

# test_ringbuffer: lib/utils/src/ringbuffer.c + gps_parser.c (for find_char)
add_executable(test_ringbuffer
    unit/test_ringbuffer.c
    ${SRC_RINGBUFFER}
    ${SRC_GPS_PARSER}
)
target_link_libraries(test_ringbuffer unity mock_common gps_stubs gps_stubs_nmea)

###############################################################################
# Module Tests (MOCKABLE modules - mock FreeRTOS/HAL)
###############################################################################

# test_gps_nmea: gps_nmea.c + ringbuffer + gps_parser utilities
add_executable(test_gps_nmea
    module/test_gps_nmea.c
    ${SRC_GPS_NMEA}
    ${SRC_GPS_PARSER}
    ${SRC_RINGBUFFER}
)
target_link_libraries(test_gps_nmea unity mock_common gps_stubs)
# WORKAROUND: GPS_NMEA_MSG_RMC is referenced in gps_nmea.c dead code (line 199)
# but not defined in NMEA_MSG_TABLE. This is tracked in tasks/gps_nmea_dead_code_cleanup.md.
# Remove this workaround after that task is completed.
target_compile_definitions(test_gps_nmea PRIVATE GPS_NMEA_MSG_RMC=0xFE)

###############################################################################
# CTest registration
###############################################################################
enable_testing()

add_test(NAME unit_parser      COMMAND test_parser)
add_test(NAME unit_ringbuffer  COMMAND test_ringbuffer)
add_test(NAME module_gps_nmea  COMMAND test_gps_nmea)
