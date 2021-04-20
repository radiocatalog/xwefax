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

#ifndef DISPLAY_H
#define DISPLAY_H   1

#include "common.h"
#include "utils.h"


/* Receive and Standby status indicators */
#define RECEIVE \
  _("<span background=\"green\" foreground=\"white\"> RECEIVE </span>")
#define STANDBY \
  _("<span background=\"lightgrey\" foreground=\"black\"> STANDBY </span>")

#define BLACK_CUT_OFF   5 /* Black cut-off percentile for normalization */
#define WHITE_CUT_OFF  40 /* White cut-off percentile for normalization */

#define SCOPE_CLEAR     2 /* Clearance in pix of scope upper and lower sides */

/* Colors used in the level gauge */
#define GAUGE_WHITE     1.0, 1.0, 1.0
#define GAUGE_RED       0.8, 0.0, 0.0
#define GAUGE_YELLOW    0.8, 0.8, 0.0
#define GAUGE_GREEN     0.0, 0.8, 0.0
#define GAUGE_GREY      0.3, 0.3, 0.3

/* Colors for the signal scope */
#define SCOPE_FOREGND       0.0, 1.0, 0.0

/* Length and multiplier of amplitude averaging window  */
#define AMPL_AVE_WIN        2
#define AMPL_AVE_MUL        1

#endif

