/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 3 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details:
 *
 *  http://www.gnu.org/copyleft/gpl.txt
 */

#ifndef CAT_H
#define CAT_H   1

#include <sys/ioctl.h>
#include <termios.h>
#include "common.h"

#define MAX_SERIAL_RETRIES  10

/* Get/Set transceiver status */
#define GET_TCVR_STATUS 0
#define SET_TCVR_STATUS 1

#endif

