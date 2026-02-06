/**
 * @file test_ringbuffer.c
 * @brief Unit tests for lib/utils/src/ringbuffer.c
 *
 * Target: lib/utils/src/ringbuffer.c (PURE module)
 * Tests: init, write, read, peek, advance, wrap-around, overflow,
 *        ringbuffer_find_char (defined in gps_parser.c)
 */

#include "unity.h"
#include "ringbuffer.h"
#include "gps_parser.h"   /* ringbuffer_find_char */
#include <string.h>

#define TEST_BUF_SIZE  64

static ringbuffer_t rb;
static char buf[TEST_BUF_SIZE];

void setUp(void) {
    memset(buf, 0, sizeof(buf));
    ringbuffer_init(&rb, buf, sizeof(buf));
}

void tearDown(void) {
    /* ringbuffer_deinit zeroes the struct */
    ringbuffer_deinit(&rb);
}

/*===========================================================================
 * Init / Basic State
 *===========================================================================*/

void test_init_empty(void) {
    TEST_ASSERT_TRUE(ringbuffer_is_empty(&rb));
    TEST_ASSERT_FALSE(ringbuffer_is_full(&rb));
    TEST_ASSERT_EQUAL(0, ringbuffer_size(&rb));
    TEST_ASSERT_EQUAL(TEST_BUF_SIZE, ringbuffer_capacity(&rb));
}

void test_init_free_size(void) {
    /* capacity 64, usable = 63 (one slot reserved) */
    TEST_ASSERT_EQUAL(TEST_BUF_SIZE - 1, ringbuffer_free_size(&rb));
}

/*===========================================================================
 * Write / Read
 *===========================================================================*/

void test_write_read_basic(void) {
    ringbuffer_write(&rb, "hello", 5);
    TEST_ASSERT_EQUAL(5, ringbuffer_size(&rb));
    TEST_ASSERT_FALSE(ringbuffer_is_empty(&rb));

    char out[8] = {0};
    size_t n = ringbuffer_read(&rb, out, 5);
    TEST_ASSERT_EQUAL(5, n);
    TEST_ASSERT_EQUAL_STRING("hello", out);
    TEST_ASSERT_TRUE(ringbuffer_is_empty(&rb));
}

void test_write_byte_read_byte(void) {
    ringbuffer_write_byte(&rb, 'A');
    ringbuffer_write_byte(&rb, 'B');
    TEST_ASSERT_EQUAL(2, ringbuffer_size(&rb));

    char c;
    TEST_ASSERT_TRUE(ringbuffer_read_byte(&rb, &c));
    TEST_ASSERT_EQUAL_CHAR('A', c);
    TEST_ASSERT_TRUE(ringbuffer_read_byte(&rb, &c));
    TEST_ASSERT_EQUAL_CHAR('B', c);
    TEST_ASSERT_FALSE(ringbuffer_read_byte(&rb, &c));
}

void test_read_empty_returns_false(void) {
    char c;
    TEST_ASSERT_FALSE(ringbuffer_read_byte(&rb, &c));
}

void test_read_partial(void) {
    ringbuffer_write(&rb, "ABC", 3);
    char out[8] = {0};
    /* Request more than available */
    size_t n = ringbuffer_read(&rb, out, 10);
    TEST_ASSERT_EQUAL(3, n);
    TEST_ASSERT_EQUAL_MEMORY("ABC", out, 3);
}

/*===========================================================================
 * Wrap-around
 *===========================================================================*/

void test_wrap_around(void) {
    /* Fill most of the buffer, read it, then write across the boundary */
    char fill[60];
    memset(fill, 'A', 60);
    ringbuffer_write(&rb, fill, 60);

    char tmp[60];
    ringbuffer_read(&rb, tmp, 60);

    /* Now tail is near the end, write wraps around */
    ringbuffer_write(&rb, "WRAP_TEST!", 10);

    char out[16] = {0};
    size_t n = ringbuffer_read(&rb, out, 10);
    TEST_ASSERT_EQUAL(10, n);
    TEST_ASSERT_EQUAL_STRING("WRAP_TEST!", out);
}

void test_peek_wrap_around(void) {
    /* Move tail to near end */
    char fill[60];
    memset(fill, 'X', 60);
    ringbuffer_write(&rb, fill, 60);
    ringbuffer_read(&rb, fill, 60);

    /* Write across wrap boundary */
    ringbuffer_write(&rb, "ABCDEFGH", 8);

    /* Peek entire data */
    char out[16] = {0};
    TEST_ASSERT_TRUE(ringbuffer_peek(&rb, out, 8, 0));
    TEST_ASSERT_EQUAL_MEMORY("ABCDEFGH", out, 8);

    /* Peek with offset */
    char c;
    TEST_ASSERT_TRUE(ringbuffer_peek(&rb, &c, 1, 5));
    TEST_ASSERT_EQUAL_CHAR('F', c);
}

/*===========================================================================
 * Overflow
 *===========================================================================*/

void test_overflow_flag(void) {
    TEST_ASSERT_FALSE(ringbuffer_is_overflow(&rb));
    TEST_ASSERT_EQUAL(0, ringbuffer_get_overflow_count(&rb));

    /* Write more than capacity (63 usable, write 63 which fits) */
    char big[63];
    memset(big, 'O', 63);
    ringbuffer_write(&rb, big, 63);
    TEST_ASSERT_FALSE(ringbuffer_is_overflow(&rb));

    /* Now buffer is full, writing 1 more byte causes overflow */
    ringbuffer_write_byte(&rb, 'X');
    TEST_ASSERT_TRUE(ringbuffer_is_overflow(&rb));
    TEST_ASSERT_TRUE(ringbuffer_get_overflow_count(&rb) > 0);
}

void test_reset_clears_overflow(void) {
    ringbuffer_write_byte(&rb, 'A');
    ringbuffer_reset(&rb);
    TEST_ASSERT_TRUE(ringbuffer_is_empty(&rb));
    TEST_ASSERT_FALSE(ringbuffer_is_overflow(&rb));
    TEST_ASSERT_EQUAL(0, ringbuffer_get_overflow_count(&rb));
}

/*===========================================================================
 * Peek
 *===========================================================================*/

void test_peek_does_not_consume(void) {
    ringbuffer_write(&rb, "DATA", 4);

    char out[8] = {0};
    TEST_ASSERT_TRUE(ringbuffer_peek(&rb, out, 4, 0));
    TEST_ASSERT_EQUAL_MEMORY("DATA", out, 4);

    /* Size unchanged after peek */
    TEST_ASSERT_EQUAL(4, ringbuffer_size(&rb));
}

void test_peek_with_offset(void) {
    ringbuffer_write(&rb, "ABCDE", 5);

    char c;
    TEST_ASSERT_TRUE(ringbuffer_peek(&rb, &c, 1, 0));
    TEST_ASSERT_EQUAL_CHAR('A', c);

    TEST_ASSERT_TRUE(ringbuffer_peek(&rb, &c, 1, 3));
    TEST_ASSERT_EQUAL_CHAR('D', c);

    TEST_ASSERT_TRUE(ringbuffer_peek(&rb, &c, 1, 4));
    TEST_ASSERT_EQUAL_CHAR('E', c);
}

void test_peek_beyond_data_fails(void) {
    ringbuffer_write(&rb, "AB", 2);
    char c;
    TEST_ASSERT_FALSE(ringbuffer_peek(&rb, &c, 1, 5));
}

void test_peek_zero_len(void) {
    char c = 'Z';
    TEST_ASSERT_TRUE(ringbuffer_peek(&rb, &c, 0, 0));
    /* c should be unchanged */
    TEST_ASSERT_EQUAL_CHAR('Z', c);
}

/*===========================================================================
 * Advance
 *===========================================================================*/

void test_advance_basic(void) {
    ringbuffer_write(&rb, "ABCDE", 5);
    TEST_ASSERT_TRUE(ringbuffer_advance(&rb, 3));
    TEST_ASSERT_EQUAL(2, ringbuffer_size(&rb));

    char out[4] = {0};
    ringbuffer_read(&rb, out, 2);
    TEST_ASSERT_EQUAL_MEMORY("DE", out, 2);
}

void test_advance_beyond_available_resets(void) {
    ringbuffer_write(&rb, "AB", 2);
    TEST_ASSERT_FALSE(ringbuffer_advance(&rb, 10));
    TEST_ASSERT_TRUE(ringbuffer_is_empty(&rb));
}

/*===========================================================================
 * ringbuffer_find_char (from gps_parser.c)
 *===========================================================================*/

void test_find_char_basic(void) {
    ringbuffer_write(&rb, "$GPGGA,test\r\n", 13);

    size_t pos;
    TEST_ASSERT_TRUE(ringbuffer_find_char(&rb, '\r', 20, &pos));
    TEST_ASSERT_EQUAL(11, pos);
}

void test_find_char_not_found(void) {
    ringbuffer_write(&rb, "no_cr_here", 10);

    size_t pos;
    TEST_ASSERT_FALSE(ringbuffer_find_char(&rb, '\r', 20, &pos));
}

void test_find_char_max_search(void) {
    ringbuffer_write(&rb, "ABCDEFGHIJ\r", 11);

    size_t pos;
    /* Search only first 5 bytes → won't find \r at position 10 */
    TEST_ASSERT_FALSE(ringbuffer_find_char(&rb, '\r', 5, &pos));

    /* Search all → finds it */
    TEST_ASSERT_TRUE(ringbuffer_find_char(&rb, '\r', 20, &pos));
    TEST_ASSERT_EQUAL(10, pos);
}

void test_find_char_first_byte(void) {
    ringbuffer_write(&rb, "$test", 5);

    size_t pos;
    TEST_ASSERT_TRUE(ringbuffer_find_char(&rb, '$', 10, &pos));
    TEST_ASSERT_EQUAL(0, pos);
}

void test_find_char_wrap_around(void) {
    /* Move tail near end, then write across boundary */
    char fill[60];
    memset(fill, 'X', 60);
    ringbuffer_write(&rb, fill, 60);
    ringbuffer_read(&rb, fill, 60);

    /* Write "$AB\r" across wrap boundary */
    ringbuffer_write(&rb, "$AB\r", 4);

    size_t pos;
    TEST_ASSERT_TRUE(ringbuffer_find_char(&rb, '\r', 10, &pos));
    TEST_ASSERT_EQUAL(3, pos);
}

/*===========================================================================
 * Runner
 *===========================================================================*/

int main(void) {
    UNITY_BEGIN();

    /* Init / State */
    RUN_TEST(test_init_empty);
    RUN_TEST(test_init_free_size);

    /* Write / Read */
    RUN_TEST(test_write_read_basic);
    RUN_TEST(test_write_byte_read_byte);
    RUN_TEST(test_read_empty_returns_false);
    RUN_TEST(test_read_partial);

    /* Wrap-around */
    RUN_TEST(test_wrap_around);
    RUN_TEST(test_peek_wrap_around);

    /* Overflow */
    RUN_TEST(test_overflow_flag);
    RUN_TEST(test_reset_clears_overflow);

    /* Peek */
    RUN_TEST(test_peek_does_not_consume);
    RUN_TEST(test_peek_with_offset);
    RUN_TEST(test_peek_beyond_data_fails);
    RUN_TEST(test_peek_zero_len);

    /* Advance */
    RUN_TEST(test_advance_basic);
    RUN_TEST(test_advance_beyond_available_resets);

    /* ringbuffer_find_char */
    RUN_TEST(test_find_char_basic);
    RUN_TEST(test_find_char_not_found);
    RUN_TEST(test_find_char_max_search);
    RUN_TEST(test_find_char_first_byte);
    RUN_TEST(test_find_char_wrap_around);

    return UNITY_END();
}
