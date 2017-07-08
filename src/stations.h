/*  stations.h
 *
 *  Stations treview functions for xwefax application
 */

/*
 *  xwefax: An application to decode Radio WEFAX signals from
 *  a Radio Receiver, through the computer's sound card.
 *
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

#ifndef STATIONS_H
#define STATIONS_H	1

#include "common.h"
#include "shared.h"
#include "support.h"
#include "utils.h"

/* Station list columns */
enum
{
  NAME_COL = 0,
  FREQ_COL,
  SDB_COL,
  RPM_COL,
  RESOL_COL,
  IOC_COL,
  PHL_COL,
  SLANT_COL,
  LIST_NUM_COLS
};

#define STATIONS_LINE_LEN		75	/* Stations file line length */
#define STATIONS_NAME_WIDTH		32	/* Stations file width in char of Name */
#define STATIONS_FREQ_WIDTH		11	/* Stations file width in char of Freq */
#define STATIONS_SDB_WIDTH		4	/* Stations file width in char of Sideband */
#define STATIONS_RPM_WIDTH		4	/* Stations file width in char of RPM */
#define STATIONS_RESOL_WIDTH	5	/* Stations file width in char of Resol */
#define STATIONS_IOC_WIDTH		4	/* Stations file width in char of IOC */
#define STATIONS_PHL_WIDTH		3	/* Stations file width in char of Phasing Lines */
#define STATIONS_SLANT_WIDTH	4	/* Stations file width in char of Deslant */

#endif
