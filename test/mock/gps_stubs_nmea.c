/**
 * @file gps_stubs_nmea.c
 * @brief NMEA parser stub (for tests that link gps_parser.c but don't need real NMEA parser)
 *
 * Used by: test_ringbuffer (needs gps_parser.c utilities but not NMEA parsing)
 * NOT used by: test_gps_nmea (links real gps_nmea.c)
 */

#include "gps_parser.h"

parse_result_t nmea_try_parse(gps_t *gps, ringbuffer_t *rb) {
    (void)gps;
    (void)rb;
    return PARSE_NOT_MINE;
}
