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

#ifndef DETECT_H
#define DETECT_H    1

#include "common.h"

/* Multiplier and floor level to bring
 * detectors output to range 0 - 255 */
#define DISCR_SCALE     3.0
#define DISCR_FLOOR     510.0

/* To keep values of detected signal in reasonable limits */
#define BILEVEL_SCALE_FACTOR    750.0

/* Detection thresholds for start and stop tones */
#define START_TONE_UP       300000
#define START_TONE_DOWN     100000
#define STOP_TONE_UP        300000
#define STOP_TONE_DOWN      100000

/* Number of Start/Stop tone periods * 60 sec
 * used in the Goertzel detector */
#define TONE_PERIOD_MULT        3600.0

/* Length of Start/Stop tone averaging window */
#define TONE_LEVEL_AVE_WIN      8

/* Length of signal averaging window */
#define SIG_AVE_WINDOW      20.0

/* Scale factors for displaying signal level in the Gauge */
#define SIG_GAUGE_SCALE     200
#define SIG_GAUGE_LEVEL1    32
#define SIG_GAUGE_LEVEL2    80

/* Scale factors for displaying stop/start level in the Gauge */
#define STOP_GAUGE_SCALE    2500
#define START_GAUGE_SCALE   2500

/* Count of calls to gauge display function
 * before the gauge display is refreshed */
#define GAUGE_COUNT     256

#endif
