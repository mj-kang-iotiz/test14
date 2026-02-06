/**
 * @file test_gps_nmea.c
 * @brief Module tests for lib/gps/gps_nmea.c
 *
 * Target: gps_nmea.c NMEA parser (MOCKABLE module)
 * Dependencies: ringbuffer.c, gps_parser.c (utilities), mock FreeRTOS/HAL
 *
 * Tests: GGA parsing, THS parsing, CRC verification,
 *        unregistered NMEA handling, error cases
 */

#include "unity.h"
#include "gps.h"
#include "gps_parser.h"
#include "nmea/nmea_fixture.h"
#include <string.h>

/*===========================================================================
 * Test fixtures
 *===========================================================================*/

static gps_t gps;

/* Event capture for handler tests */
static gps_event_t last_event;
static int event_count;

static void test_event_handler(gps_t *g, const gps_event_t *event) {
    (void)g;
    memcpy(&last_event, event, sizeof(gps_event_t));
    event_count++;
}

void setUp(void) {
    memset(&gps, 0, sizeof(gps_t));
    memset(&last_event, 0, sizeof(gps_event_t));
    event_count = 0;

    /* Initialize rx_buf ringbuffer */
    ringbuffer_init(&gps.rx_buf, gps.rx_buf_mem, sizeof(gps.rx_buf_mem));

    /* Set event handler */
    gps.handler = test_event_handler;
}

void tearDown(void) {
    /* nothing to free (no OS resources in test) */
}

/*===========================================================================
 * Helper: feed data into ringbuffer and parse
 *===========================================================================*/

static parse_result_t feed_and_parse(const char *data, size_t len) {
    ringbuffer_write(&gps.rx_buf, data, len);
    return nmea_try_parse(&gps, &gps.rx_buf);
}

/*===========================================================================
 * GGA Parsing
 *===========================================================================*/

void test_gga_basic_parse(void) {
    parse_result_t r = feed_and_parse(GGA_BASIC, strlen(GGA_BASIC));
    TEST_ASSERT_EQUAL(PARSE_OK, r);

    /* Time: 09:27:25 */
    TEST_ASSERT_EQUAL_UINT8(9, gps.nmea_data.gga.hour);
    TEST_ASSERT_EQUAL_UINT8(27, gps.nmea_data.gga.min);
    TEST_ASSERT_EQUAL_UINT8(25, gps.nmea_data.gga.sec);

    /* Position: 47°17.11399'N → 47.28523... */
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 47.2852, gps.nmea_data.gga.lat);
    TEST_ASSERT_EQUAL_CHAR('N', gps.nmea_data.gga.ns);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 8.5653, gps.nmea_data.gga.lon);
    TEST_ASSERT_EQUAL_CHAR('E', gps.nmea_data.gga.ew);

    /* Fix, satellites, HDOP, altitude */
    TEST_ASSERT_EQUAL(GPS_FIX_GPS, gps.nmea_data.gga.fix);
    TEST_ASSERT_EQUAL_UINT8(8, gps.nmea_data.gga.sat_num);
    TEST_ASSERT_DOUBLE_WITHIN(0.01, 1.01, gps.nmea_data.gga.hdop);
    TEST_ASSERT_DOUBLE_WITHIN(0.1, 499.6, gps.nmea_data.gga.alt);
}

void test_gga_rtk_fix(void) {
    parse_result_t r = feed_and_parse(GGA_RTK_FIX, strlen(GGA_RTK_FIX));
    TEST_ASSERT_EQUAL(PARSE_OK, r);
    TEST_ASSERT_EQUAL(GPS_FIX_RTK_FIX, gps.nmea_data.gga.fix);
    TEST_ASSERT_EQUAL_UINT8(12, gps.nmea_data.gga.sat_num);
}

void test_gga_rtk_float(void) {
    parse_result_t r = feed_and_parse(GGA_RTK_FLOAT, strlen(GGA_RTK_FLOAT));
    TEST_ASSERT_EQUAL(PARSE_OK, r);
    TEST_ASSERT_EQUAL(GPS_FIX_RTK_FLOAT, gps.nmea_data.gga.fix);
}

void test_gga_no_fix(void) {
    parse_result_t r = feed_and_parse(GGA_NO_FIX, strlen(GGA_NO_FIX));
    TEST_ASSERT_EQUAL(PARSE_OK, r);
    TEST_ASSERT_EQUAL(GPS_FIX_INVALID, gps.nmea_data.gga.fix);
    TEST_ASSERT_EQUAL_UINT8(0, gps.nmea_data.gga.sat_num);
}

void test_gga_south_west(void) {
    parse_result_t r = feed_and_parse(GGA_SOUTH_WEST, strlen(GGA_SOUTH_WEST));
    TEST_ASSERT_EQUAL(PARSE_OK, r);
    TEST_ASSERT_EQUAL_CHAR('S', gps.nmea_data.gga.ns);
    TEST_ASSERT_EQUAL_CHAR('W', gps.nmea_data.gga.ew);
    TEST_ASSERT_TRUE(gps.nmea_data.gga.lat > 0);  /* lat value is positive, direction is S */
}

void test_gga_updates_common_data(void) {
    /* First parse: fix changes from 0 to 1 → fix_changed = true */
    feed_and_parse(GGA_BASIC, strlen(GGA_BASIC));
    TEST_ASSERT_EQUAL(GPS_FIX_GPS, gps.data.status.fix_type);
    TEST_ASSERT_TRUE(gps.data.status.fix_changed);
    TEST_ASSERT_DOUBLE_WITHIN(0.01, 1.01, gps.data.status.hdop);

    /* Second parse with same fix → fix_changed = false */
    ringbuffer_reset(&gps.rx_buf);
    feed_and_parse(GGA_BASIC, strlen(GGA_BASIC));
    TEST_ASSERT_FALSE(gps.data.status.fix_changed);
}

void test_gga_ringbuffer_consumed(void) {
    feed_and_parse(GGA_BASIC, strlen(GGA_BASIC));
    /* Ringbuffer should be empty after successful parse */
    TEST_ASSERT_TRUE(ringbuffer_is_empty(&gps.rx_buf));
}

/*===========================================================================
 * THS Parsing
 *===========================================================================*/

void test_ths_parse(void) {
    parse_result_t r = feed_and_parse(THS_VALID, strlen(THS_VALID));
    TEST_ASSERT_EQUAL(PARSE_OK, r);
    TEST_ASSERT_DOUBLE_WITHIN(0.1, 270.5, gps.nmea_data.ths.heading);
    TEST_ASSERT_EQUAL(GPS_THS_MODE_AUTO, gps.nmea_data.ths.mode);
}

void test_ths_invalid_mode(void) {
    parse_result_t r = feed_and_parse(THS_INVALID_MODE, strlen(THS_INVALID_MODE));
    TEST_ASSERT_EQUAL(PARSE_OK, r);
    TEST_ASSERT_EQUAL(GPS_THS_MODE_INVALID, gps.nmea_data.ths.mode);
}

void test_ths_updates_common_data(void) {
    feed_and_parse(THS_VALID, strlen(THS_VALID));
    TEST_ASSERT_DOUBLE_WITHIN(0.1, 270.5, gps.data.heading.heading);
    TEST_ASSERT_EQUAL('A', gps.data.heading.mode);
}

/*===========================================================================
 * CRC / Validation
 *===========================================================================*/

void test_bad_crc_returns_invalid(void) {
    parse_result_t r = feed_and_parse(GGA_BAD_CRC, strlen(GGA_BAD_CRC));
    TEST_ASSERT_EQUAL(PARSE_INVALID, r);
    TEST_ASSERT_EQUAL(1, gps.parser_ctx.stats.crc_errors);
}

void test_too_few_fields_returns_invalid(void) {
    parse_result_t r = feed_and_parse(GGA_TOO_FEW_FIELDS, strlen(GGA_TOO_FEW_FIELDS));
    TEST_ASSERT_EQUAL(PARSE_INVALID, r);
}

/*===========================================================================
 * Unregistered NMEA (NOT_MINE → next parser tries)
 *===========================================================================*/

void test_unregistered_nmea_returns_not_mine(void) {
    /* GSV is not in NMEA_MSG_TABLE → PARSE_NOT_MINE */
    parse_result_t r = feed_and_parse(GSV_UNREGISTERED, strlen(GSV_UNREGISTERED));
    TEST_ASSERT_EQUAL(PARSE_NOT_MINE, r);
}

/*===========================================================================
 * Incomplete data
 *===========================================================================*/

void test_incomplete_returns_need_more(void) {
    parse_result_t r = feed_and_parse(GGA_INCOMPLETE, strlen(GGA_INCOMPLETE));
    TEST_ASSERT_EQUAL(PARSE_NEED_MORE, r);
}

void test_empty_buffer_returns_need_more(void) {
    /* Empty ringbuffer */
    parse_result_t r = nmea_try_parse(&gps, &gps.rx_buf);
    TEST_ASSERT_EQUAL(PARSE_NEED_MORE, r);
}

void test_first_byte_not_dollar_returns_not_mine(void) {
    const char *data = "GPGGA,092725\r\n";
    parse_result_t r = feed_and_parse(data, strlen(data));
    TEST_ASSERT_EQUAL(PARSE_NOT_MINE, r);
}

void test_non_nmea_talker_returns_not_mine(void) {
    /* $XX is not GP/GN/GL/GA/GB → NOT_MINE (could be Unicore $command) */
    const char *data = "$XXYYY,test*00\r\n";
    parse_result_t r = feed_and_parse(data, strlen(data));
    TEST_ASSERT_EQUAL(PARSE_NOT_MINE, r);
}

/*===========================================================================
 * Statistics
 *===========================================================================*/

void test_stats_increment_on_success(void) {
    TEST_ASSERT_EQUAL(0, gps.parser_ctx.stats.nmea_packets);
    feed_and_parse(GGA_BASIC, strlen(GGA_BASIC));
    TEST_ASSERT_EQUAL(1, gps.parser_ctx.stats.nmea_packets);
}

/*===========================================================================
 * Event handler
 *===========================================================================*/

void test_gga_fix_change_fires_event(void) {
    /* fix 0 → 1: should fire GPS_EVENT_FIX_UPDATED */
    feed_and_parse(GGA_BASIC, strlen(GGA_BASIC));
    TEST_ASSERT_EQUAL(1, event_count);
    TEST_ASSERT_EQUAL(GPS_EVENT_FIX_UPDATED, last_event.type);
    TEST_ASSERT_EQUAL(GPS_PROTOCOL_NMEA, last_event.protocol);
}

void test_gga_same_fix_no_event(void) {
    /* First parse triggers event */
    feed_and_parse(GGA_BASIC, strlen(GGA_BASIC));
    TEST_ASSERT_EQUAL(1, event_count);

    /* Same fix type again → no fix event */
    ringbuffer_reset(&gps.rx_buf);
    event_count = 0;
    feed_and_parse(GGA_BASIC, strlen(GGA_BASIC));
    TEST_ASSERT_EQUAL(0, event_count);
}

void test_ths_fires_heading_event(void) {
    feed_and_parse(THS_VALID, strlen(THS_VALID));
    TEST_ASSERT_EQUAL(1, event_count);
    TEST_ASSERT_EQUAL(GPS_EVENT_HEADING_UPDATED, last_event.type);
}

/*===========================================================================
 * Sequential parsing (multiple messages in buffer)
 *===========================================================================*/

void test_sequential_gga_then_ths(void) {
    /* Two messages in sequence */
    ringbuffer_write(&gps.rx_buf, GGA_BASIC, strlen(GGA_BASIC));
    ringbuffer_write(&gps.rx_buf, THS_VALID, strlen(THS_VALID));

    parse_result_t r1 = nmea_try_parse(&gps, &gps.rx_buf);
    TEST_ASSERT_EQUAL(PARSE_OK, r1);
    TEST_ASSERT_EQUAL(GPS_FIX_GPS, gps.nmea_data.gga.fix);

    parse_result_t r2 = nmea_try_parse(&gps, &gps.rx_buf);
    TEST_ASSERT_EQUAL(PARSE_OK, r2);
    TEST_ASSERT_DOUBLE_WITHIN(0.1, 270.5, gps.nmea_data.ths.heading);

    TEST_ASSERT_TRUE(ringbuffer_is_empty(&gps.rx_buf));
}

/*===========================================================================
 * Runner
 *===========================================================================*/

int main(void) {
    UNITY_BEGIN();

    /* GGA parsing */
    RUN_TEST(test_gga_basic_parse);
    RUN_TEST(test_gga_rtk_fix);
    RUN_TEST(test_gga_rtk_float);
    RUN_TEST(test_gga_no_fix);
    RUN_TEST(test_gga_south_west);
    RUN_TEST(test_gga_updates_common_data);
    RUN_TEST(test_gga_ringbuffer_consumed);

    /* THS parsing */
    RUN_TEST(test_ths_parse);
    RUN_TEST(test_ths_invalid_mode);
    RUN_TEST(test_ths_updates_common_data);

    /* CRC / Validation */
    RUN_TEST(test_bad_crc_returns_invalid);
    RUN_TEST(test_too_few_fields_returns_invalid);

    /* Unregistered NMEA */
    RUN_TEST(test_unregistered_nmea_returns_not_mine);

    /* Incomplete data */
    RUN_TEST(test_incomplete_returns_need_more);
    RUN_TEST(test_empty_buffer_returns_need_more);
    RUN_TEST(test_first_byte_not_dollar_returns_not_mine);
    RUN_TEST(test_non_nmea_talker_returns_not_mine);

    /* Statistics */
    RUN_TEST(test_stats_increment_on_success);

    /* Event handler */
    RUN_TEST(test_gga_fix_change_fires_event);
    RUN_TEST(test_gga_same_fix_no_event);
    RUN_TEST(test_ths_fires_heading_event);

    /* Sequential */
    RUN_TEST(test_sequential_gga_then_ths);

    return UNITY_END();
}
