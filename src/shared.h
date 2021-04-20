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

#ifndef SHARED_H
#define SHARED_H    1

#include "common.h"
#include "sound.h"

/* Runtime config data */
extern rc_data_t rc_data;

/* Pixel buffer */
extern guchar *pixel_buf;

/* Pixel buffer for display */
extern GdkPixbuf *wefax_pixbuf;

/* Buffer for pixels of one image line */
extern unsigned char *line_buffer;
extern int line_count;
extern int linebuff_input, linebuff_output;

/* dft in/out buffers */
extern int
  *dft_in_r,
  *dft_out_r,
  *dft_out_i;

/* Pixbuf rowstride and num of channels */
extern gint
  rowstride,
  n_channels;

/* Tree list store */
extern GtkListStore *stations_list_store;
extern GtkTreeView  *stations_treeview;

extern GtkWidget
  *stations_window,      /* Stations list window */
  *popup_menu,           /* Popup main menu */
  *main_window,          /* xwefax's top window */
  *scope_drawingarea,    /* Signal scope widget */
  *spectrum_drawingarea, /* Signal spectrum widget */
  *level_gauge,          /* The vertical level gauge in control frame */
  *text_scroller,        /* Text view scroller */
  *wefax_drawingarea;    /* Drawingarea to display WEFAX image */

/* Gtk builders for some above that need to be global */
extern GtkBuilder
  *stations_window_builder,
  *popup_menu_builder,
  *main_window_builder;

/* Pixbuffer for waterfall */
extern GdkPixbuf *wfall_pixbuf;
extern guchar    *wfall_pixels;
extern gint
  wfall_rowstride,
  wfall_n_channels,
  wfall_width,
  wfall_height;

/* Signal scope size */
extern gint scope_width, scope_height;

/* Values needed to plot the level gauge */
extern int gauge_input, gauge_level1, gauge_level2;

/* Text buffer for text view */
extern GtkTextBuffer *text_buffer;

/* Pixbuf rowstride and num of channels */
extern gint
  rowstride,
  n_channels;

/* Image files name  */
extern char image_file[MAX_FILE_NAME];

/* Buffer for reading in DSP data */
extern short *dsp_buffer;

/* Average value of DFT bins */
extern int *bin_ave;

/* What action the WEFAX decoder should enter */
extern int wefax_action;

/* Fm Detector function pointer */
extern gboolean ( *FM_Detector ) ( unsigned char *level );

/* Semaphore to control async IQ data transfer */
extern sem_t pback_semaphore;

#define SCOPE_BACKGND   0.0, 0.3, 0.0
#define PIXBUF_BACKGND  0xb0b0b0ff

#endif

