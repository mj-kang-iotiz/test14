/**
 * @file nmea_fixture.h
 * @brief NMEA test data (fixture)
 *
 * 모든 테스트 데이터는 static const로 정의.
 * 사용하지 않는 데이터는 컴파일러가 자동 제거.
 *
 * 네이밍 규칙: {MSG_TYPE}_{CASE_NAME}
 *   예: GGA_BASIC, GGA_RTK_FIX, THS_VALID
 */
#ifndef NMEA_FIXTURE_H
#define NMEA_FIXTURE_H

/*===========================================================================
 * GGA (Global Positioning System Fix Data)
 * $xxGGA,time,lat,NS,lon,EW,quality,numSV,HDOP,alt,M,sep,M,diffAge,diffStation*cs
 *===========================================================================*/

/* 정상: GPS fix, 8 satellites, HDOP 1.01, alt 499.6m */
static const char GGA_BASIC[] =
    "$GPGGA,092725.00,4717.11399,N,00833.91590,E,1,08,1.01,499.6,M,48.0,M,,*5B\r\n";

/* RTK Fixed (fix=4), 12 satellites */
static const char GGA_RTK_FIX[] =
    "$GNGGA,061545.00,3723.71010,N,12657.89710,E,4,12,0.80,52.3,M,19.8,M,1.0,0000*56\r\n";

/* RTK Float (fix=5) */
static const char GGA_RTK_FLOAT[] =
    "$GNGGA,061545.00,3723.71010,N,12657.89710,E,5,10,1.20,52.3,M,19.8,M,2.0,0000*5D\r\n";

/* No fix (fix=0), empty fields */
static const char GGA_NO_FIX[] =
    "$GPGGA,235959.00,,,,,0,00,99.99,,M,,M,,*67\r\n";

/* 남반구 + 서반구 (S, W) */
static const char GGA_SOUTH_WEST[] =
    "$GPGGA,120000.00,3348.54000,S,07034.12000,W,1,06,1.50,100.0,M,30.0,M,,*5E\r\n";

/*===========================================================================
 * THS (True Heading and Status)
 * $xxTHS,heading,mode*cs
 *===========================================================================*/

/* 정상: heading 270.5, Auto mode */
static const char THS_VALID[] =
    "$GPTHS,270.50,A*07\r\n";

/* Invalid mode */
static const char THS_INVALID_MODE[] =
    "$GPTHS,0.00,V*10\r\n";

/*===========================================================================
 * 미등록 NMEA (skip 테스트용)
 *===========================================================================*/

/* GSV: NMEA_MSG_TABLE에 미등록 → skip 대상 */
static const char GSV_UNREGISTERED[] =
    "$GPGSV,3,1,12,01,05,060,15,02,17,259,20,04,36,147,30,05,12,095,*7C\r\n";

/*===========================================================================
 * 혼합 스트림 (연속 파싱 테스트용)
 *===========================================================================*/

/* GSV(미등록) + GGA(등록) 연속 */
static const char STREAM_GSV_THEN_GGA[] =
    "$GPGSV,3,1,12,01,05,060,15,02,17,259,20,04,36,147,30,05,12,095,*7C\r\n"
    "$GPGGA,092725.00,4717.11399,N,00833.91590,E,1,08,1.01,499.6,M,48.0,M,,*5B\r\n";

/*===========================================================================
 * 에러 케이스
 *===========================================================================*/

/* CRC 오류 (마지막 2자리 변경) */
static const char GGA_BAD_CRC[] =
    "$GPGGA,092725.00,4717.11399,N,00833.91590,E,1,08,1.01,499.6,M,48.0,M,,*FF\r\n";

/* 불완전 (CR/LF 없음) */
static const char GGA_INCOMPLETE[] =
    "$GPGGA,092725.00,4717.11399,N,00833";

/* 필드 수 부족 (5개 필드만) */
static const char GGA_TOO_FEW_FIELDS[] =
    "$GPGGA,092725.00,4717.11399,N,00833.91590*07\r\n";

#endif /* NMEA_FIXTURE_H */
