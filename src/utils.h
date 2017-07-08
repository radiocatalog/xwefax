/*	utils.h
 *
 *  Utility functions for xwefax application
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

#ifndef UTILS_H
#define UTILS_H		1

#include "shared.h"
#include "interface.h"
#include "support.h"
#include "perseus.h"
#include "sound.h"

/* Choices of RPM values in menu */
#define RPM60		60
#define RPM90		90
#define RPM100		100
#define RPM120		120
#define RPM180		180
#define RPM240		240
#define NUM_RPM		6

/* Choices of Pix/line values in menu */
#define PIX600		600
#define PIX1200		1200
#define NUM_PIX		2

/* Choices of IOC values in menu */
#define IOC288		288
#define IOC576		576
#define NUM_IOC		2

/* Choices of number Phasing lines in menu */
#define PHL10		10
#define PHL20		20
#define PHL40		40
#define PHL60		60
#define NUM_PHL		4

/* Choices of image enhancement algorithm */
enum
{
  IME0 = 0,
  IME1,
  IME2,
  NUM_IME
};

#endif

