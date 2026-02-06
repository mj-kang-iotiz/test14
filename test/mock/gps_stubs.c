/**
 * @file gps_stubs.c
 * @brief GPS parser stubs for module testing
 *
 * gps_parser.c links against all protocol parsers.
 * When testing individual parsers (e.g., NMEA only),
 * these stubs satisfy the linker for unused parsers.
 */

#include "gps_parser.h"

parse_result_t unicore_ascii_try_parse(gps_t *gps, ringbuffer_t *rb) {
    (void)gps; (void)rb;
    return PARSE_NOT_MINE;
}

parse_result_t unicore_bin_try_parse(gps_t *gps, ringbuffer_t *rb) {
    (void)gps; (void)rb;
    return PARSE_NOT_MINE;
}

parse_result_t rtcm_try_parse(gps_t *gps, ringbuffer_t *rb) {
    (void)gps; (void)rb;
    return PARSE_NOT_MINE;
}
