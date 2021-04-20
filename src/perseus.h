/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details:
 *
 *  http://www.gnu.org/copyleft/gpl.txt
 */

#ifndef PERSEUS_H
#define PERSEUS_H   1

#include "common.h"
#include "filters.h"

/* Async buffer size = 6 * 1024 I/Q samples */
#define PERSEUS_ASYNC_BUF_SIZE  6144

/* Perseus I/Q data buffer length */
#define PERSEUS_BUFFER_LEN      32768

/* Perseus sample rate */
#define PERSEUS_SAMPLE_RATE     125000

/* Demodulator bandwidth (1.5kHz) */
#define PERSEUS_DEMOD_BANDW     1400.0

/* Weaver phasing frequency. This should have been 1900 Hz,
 * the center of the WEFAX FM deviation (1500 - 2300Hz) but
 * the scheme used in xwefax to generate the Weaver phasing
 * signal requires this value to be an integer divisor of
 * the 48000Hz sampling rate. It is compensated below */
#define PERSEUS_WEAVER_FREQ     1920

/* This offset in Hz from the designated frequency
 * of WEFAX stations is needed to tune an SDR type
 * receiver to the carrier frequency of WEFAX stations.
 * The value should have been 1900Hz but it is changed
 * to compensate for the deviation in the value above */
#define WEFAX_CARRIER_OFFSET    1920

/* Perseus receiver device index */
#define PERSEUS_DEVICE_INDEX    0

/* Debug printout level, 0 - 3 (Max debug level) */
#define PERSEUS_DEBUG_LEVEL     0

/* ADAGC log of scaling thresholds,
 * to bring attenuators in or out */
#define PERSEUS_ATT_UP      75.0
#define PERSEUS_ATT_DOWN    55.0

#define PERSEUS_INITIAL_SETTINGS    0x10
#define PERSEUS_CLEAR_SETTING       0x80
#define PERSEUS_ATTEN_SETTINGS      0x03
#define PERSEUS_ATTEN_10DB          0x01
#define PERSEUS_ATTEN_20DB          0x02

/* Audio AGC reference Audio level and decay rate */
#define ADAGC_REF_LEVEL     25000.0
#define ADAGC_DECAY         0.99995

/* This union/struct is suggested in the Perseus API */
typedef union data
{
  struct
  {
    int32_t i;
    int32_t q;
  } iq;

  struct
  {
    uint8_t     i1;
    uint8_t     i2;
    uint8_t     i3;
    uint8_t     i4;
    uint8_t     q1;
    uint8_t     q2;
    uint8_t     q3;
    uint8_t     q4;
  } iq_data;
//  } __attribute__((__packed__)) iq_data;

} iq_sample;

#endif

