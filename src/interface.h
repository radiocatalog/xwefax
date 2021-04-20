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

#ifndef INTERFACE_H
#define INTERFACE_H     1

#include "common.h"

/* Gtk Builder object ids */
#define ERROR_DIALOG_IDS \
"error_dialog", \
"error_quit_button", \
"error_ok_button", \
"error_message", \
NULL

#define POPUP_MENU_IDS \
"popup_menu", \
"station_list", \
"item2_menu", \
"zerocrossing", \
"bilevel", \
"rpm", \
"rpm_menu", \
"rpm60", \
"rpm90", \
"rpm100", \
"rpm120", \
"rpm180", \
"rpm240", \
"resolution", \
"resolution_menu", \
"pix600", \
"pix1200", \
"ioc_value", \
"ioc_value_menu", \
"ioc288", \
"ioc576", \
"phasing_lines", \
"phasing_lines_menu", \
"phl10", \
"phl20", \
"phl40", \
"phl60", \
"in_image_phasing", \
"enhance_image", \
"enhance_image_menu", \
"ime0", \
"ime1", \
"ime2", \
"item1_menu", \
"jpeg", \
"pgm", \
"both", \
"capture_setup", \
"quit", \
NULL

#define QUIT_DIALOG_IDS \
"quit_dialog", \
"quit_cancel_button", \
NULL

#define MAIN_WINDOW_IDS \
"main_window", \
"slant_align", \
"image_scrolledwindow", \
"image_viewport", \
"wefax_drawingarea", \
"image_label", \
"scope_drawingarea", \
"scope_label", \
"spectrum_drawingarea", \
"start_togglebutton", \
"rcve_status", \
"skip_button", \
"save_checkbutton", \
"deslant_spinbutton", \
"start_icon", \
"sync_icon", \
"decode_icon", \
"save_icon", \
"gauge_drawingarea", \
"text_scrolledwindow", \
"textview", \
NULL

#define STATIONS_WINDOW_IDS \
"stations_window", \
"stations_treeview", \
"down_button", \
"up_button", \
"delete_button", \
"new_button", \
"save_button", \
NULL

#endif

