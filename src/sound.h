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

#ifndef SOUND_H
#define SOUND_H     1

#include "shared.h"

#define PERIOD_SIZE     4096    /* PCM period size */
#define NUM_PERIODS        4    /* Number of periods */
#define SND_PCM_ACCESS  SND_PCM_ACCESS_RW_INTERLEAVED
#define SND_PCM_FORMAT  SND_PCM_FORMAT_S16_LE
#define EXACT_VAL       0
#define PCM_OPEN_MODE   0

#endif

