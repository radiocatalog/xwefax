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

#ifndef WEFAX_H
#define WEFAX_H 1

#include "shared.h"
#include "display.h"

/* Pixel value threshold for bilevel (0/255) image */
#define BILEVEL_THRESHOLD       160

/* Minimum in-image sync pulse level
 * before correction is applied */
#define INIMAGE_SYNC_THRESHOLD  -150

/* Sync correction allowed error range */
#define SYNC_CORRECT_RANGE      5

#define INIMAGE_PHASING_RANGE   80

#endif
