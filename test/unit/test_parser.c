/**
 * @file test_parser.c
 * @brief Unit tests for lib/parser/parser.c
 *
 * Target: lib/parser/parser.c (PURE module, no HAL dependency)
 * Tests: parse_int32, parse_uint32, parse_hex, parse_double,
 *        parse_char, parse_string, parse_string_quoted
 */

#include "unity.h"
#include "parser.h"
#include <string.h>

void setUp(void) { }
void tearDown(void) { }

/*===========================================================================
 * parse_int32
 *===========================================================================*/

void test_parse_int32_basic(void) {
    const char *p = ",123,";
    int32_t val = parse_int32(&p);
    TEST_ASSERT_EQUAL_INT32(123, val);
    TEST_ASSERT_EQUAL_CHAR(',', *p);
}

void test_parse_int32_negative(void) {
    const char *p = ",-456,";
    int32_t val = parse_int32(&p);
    TEST_ASSERT_EQUAL_INT32(-456, val);
}

void test_parse_int32_zero(void) {
    const char *p = ",0,";
    int32_t val = parse_int32(&p);
    TEST_ASSERT_EQUAL_INT32(0, val);
}

void test_parse_int32_no_leading_comma(void) {
    const char *p = "789";
    int32_t val = parse_int32(&p);
    TEST_ASSERT_EQUAL_INT32(789, val);
}

void test_parse_int32_empty_field(void) {
    /* comma followed by comma = empty field → 0 */
    const char *p = ",,";
    int32_t val = parse_int32(&p);
    TEST_ASSERT_EQUAL_INT32(0, val);
}

/*===========================================================================
 * parse_uint32
 *===========================================================================*/

void test_parse_uint32_basic(void) {
    const char *p = ",42,";
    uint32_t val = parse_uint32(&p);
    TEST_ASSERT_EQUAL_UINT32(42, val);
}

void test_parse_uint32_large(void) {
    const char *p = "4294967295";   /* UINT32_MAX */
    uint32_t val = parse_uint32(&p);
    TEST_ASSERT_EQUAL_UINT32(4294967295U, val);
}

/*===========================================================================
 * parse_hex
 *===========================================================================*/

void test_parse_hex_uppercase(void) {
    const char *p = ",5B";
    uint32_t val = parse_hex(&p);
    TEST_ASSERT_EQUAL_UINT32(0x5B, val);
}

void test_parse_hex_lowercase(void) {
    const char *p = ",ff";
    uint32_t val = parse_hex(&p);
    TEST_ASSERT_EQUAL_UINT32(0xFF, val);
}

void test_parse_hex_mixed(void) {
    const char *p = "aB12";
    uint32_t val = parse_hex(&p);
    TEST_ASSERT_EQUAL_UINT32(0xAB12, val);
}

/*===========================================================================
 * parse_double
 *===========================================================================*/

void test_parse_double_basic(void) {
    const char *p = ",499.6,";
    double val = parse_double(&p);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 499.6, val);
}

void test_parse_double_negative(void) {
    const char *p = ",-12.345,";
    double val = parse_double(&p);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, -12.345, val);
}

void test_parse_double_integer_only(void) {
    const char *p = ",100,";
    double val = parse_double(&p);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 100.0, val);
}

void test_parse_double_empty_field(void) {
    const char *p = ",,";
    double val = parse_double(&p);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 0.0, val);
}

void test_parse_double_nmea_lat(void) {
    /* NMEA latitude: 4717.11399 = 47 degrees, 17.11399 minutes */
    const char *p = ",4717.11399,";
    double val = parse_double(&p);
    TEST_ASSERT_DOUBLE_WITHIN(0.00001, 4717.11399, val);
}

/*===========================================================================
 * parse_char
 *===========================================================================*/

void test_parse_char_basic(void) {
    const char *p = ",N,";
    char ch = parse_char(&p);
    TEST_ASSERT_EQUAL_CHAR('N', ch);
}

void test_parse_char_empty_field(void) {
    const char *p = ",,";
    char ch = parse_char(&p);
    TEST_ASSERT_EQUAL_CHAR('\0', ch);
}

void test_parse_char_lowercase(void) {
    const char *p = ",a,";
    char ch = parse_char(&p);
    TEST_ASSERT_EQUAL_CHAR('a', ch);
}

void test_parse_char_digit_ignored(void) {
    /* parse_char uses PARSER_IS_CHAR which only accepts a-z, A-Z */
    const char *p = ",5,";
    char ch = parse_char(&p);
    TEST_ASSERT_EQUAL_CHAR('\0', ch);
}

/*===========================================================================
 * parse_string
 *===========================================================================*/

void test_parse_string_nmea_header(void) {
    /* NMEA starts with $ then field text */
    const char *p = "$GPGGA,092725.00";
    char buf[20] = {0};
    uint8_t result = parse_string(&p, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_UINT8(1, result);
    TEST_ASSERT_EQUAL_STRING("GPGGA", buf);
}

void test_parse_string_with_comma(void) {
    const char *p = ",hello,world";
    char buf[20] = {0};
    parse_string(&p, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("hello", buf);
}

void test_parse_string_null_dst(void) {
    const char *p = ",test";
    uint8_t result = parse_string(&p, NULL, 0);
    TEST_ASSERT_EQUAL_UINT8(0, result);
}

void test_parse_string_truncate(void) {
    const char *p = ",longstring,";
    char buf[5] = {0};
    parse_string(&p, buf, sizeof(buf));
    /* len=5 → actual max is 4 chars + null */
    TEST_ASSERT_EQUAL_STRING("long", buf);
}

/*===========================================================================
 * parse_string_quoted (GSM AT)
 *===========================================================================*/

void test_parse_string_quoted_basic(void) {
    const char *p = ",\"hello world\",";
    char buf[32] = {0};
    parse_string_quoted(&p, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("hello world", buf);
}

void test_parse_string_quoted_empty(void) {
    const char *p = ",\"\",";
    char buf[32] = {0};
    parse_string_quoted(&p, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("", buf);
}

/*===========================================================================
 * NMEA GGA full-sentence sequential parse
 *===========================================================================*/

void test_parse_gga_sentence(void) {
    /* Simulate parsing a GGA sentence field by field like the original parser_test.c */
    const char *p = "$GPGGA,092725.00,4717.11399,N,00833.91590,E,1,08,1.01,499.6,M,48.0,M,,*5B\r\n";

    char msg_id[20] = {0};
    parse_string(&p, msg_id, sizeof(msg_id));
    TEST_ASSERT_EQUAL_STRING("GPGGA", msg_id);

    double time_val = parse_double(&p);
    TEST_ASSERT_DOUBLE_WITHIN(0.01, 92725.00, time_val);

    double lat = parse_double(&p);
    TEST_ASSERT_DOUBLE_WITHIN(0.00001, 4717.11399, lat);

    char ns = parse_char(&p);
    TEST_ASSERT_EQUAL_CHAR('N', ns);

    double lon = parse_double(&p);
    TEST_ASSERT_DOUBLE_WITHIN(0.00001, 833.91590, lon);

    char ew = parse_char(&p);
    TEST_ASSERT_EQUAL_CHAR('E', ew);

    int32_t fix = parse_int32(&p);
    TEST_ASSERT_EQUAL_INT32(1, fix);

    int32_t sat = parse_int32(&p);
    TEST_ASSERT_EQUAL_INT32(8, sat);

    double hdop = parse_double(&p);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 1.01, hdop);

    double alt = parse_double(&p);
    TEST_ASSERT_DOUBLE_WITHIN(0.1, 499.6, alt);
}

/*===========================================================================
 * Runner
 *===========================================================================*/

int main(void) {
    UNITY_BEGIN();

    /* parse_int32 */
    RUN_TEST(test_parse_int32_basic);
    RUN_TEST(test_parse_int32_negative);
    RUN_TEST(test_parse_int32_zero);
    RUN_TEST(test_parse_int32_no_leading_comma);
    RUN_TEST(test_parse_int32_empty_field);

    /* parse_uint32 */
    RUN_TEST(test_parse_uint32_basic);
    RUN_TEST(test_parse_uint32_large);

    /* parse_hex */
    RUN_TEST(test_parse_hex_uppercase);
    RUN_TEST(test_parse_hex_lowercase);
    RUN_TEST(test_parse_hex_mixed);

    /* parse_double */
    RUN_TEST(test_parse_double_basic);
    RUN_TEST(test_parse_double_negative);
    RUN_TEST(test_parse_double_integer_only);
    RUN_TEST(test_parse_double_empty_field);
    RUN_TEST(test_parse_double_nmea_lat);

    /* parse_char */
    RUN_TEST(test_parse_char_basic);
    RUN_TEST(test_parse_char_empty_field);
    RUN_TEST(test_parse_char_lowercase);
    RUN_TEST(test_parse_char_digit_ignored);

    /* parse_string */
    RUN_TEST(test_parse_string_nmea_header);
    RUN_TEST(test_parse_string_with_comma);
    RUN_TEST(test_parse_string_null_dst);
    RUN_TEST(test_parse_string_truncate);

    /* parse_string_quoted */
    RUN_TEST(test_parse_string_quoted_basic);
    RUN_TEST(test_parse_string_quoted_empty);

    /* Integration: GGA sentence */
    RUN_TEST(test_parse_gga_sentence);

    return UNITY_END();
}
