/**
 * @file mock_common.c
 * @brief Common mock/stub implementations for host testing
 *
 * Provides:
 * - mock_tick_count: configurable tick counter
 * - dev_assert_failed: test-friendly assert (abort instead of infinite loop)
 * - GPS parser stubs for unneeded parsers
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/*===========================================================================
 * Mock tick counter (used by FreeRTOS and HAL stubs)
 *===========================================================================*/
uint32_t mock_tick_count = 0;

/*===========================================================================
 * dev_assert replacement (abort instead of while(1))
 *===========================================================================*/
void dev_assert_failed(const char *file, int line, const char *expr, const char *msg) {
    fprintf(stderr, "\n[ASSERT FAILED] %s:%d\n", file, line);
    if (expr) fprintf(stderr, "  expr: %s\n", expr);
    if (msg)  fprintf(stderr, "  msg:  %s\n", msg);
    abort();
}
