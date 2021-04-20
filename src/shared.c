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

#include "shared.h"

/* Runtime config data */
rc_data_t rc_data;

/* Pixel buffer */
guchar *pixel_buf;

/* Pixel buffer for display */
GdkPixbuf *wefax_pixbuf = NULL;

/* Buffer for pixels of one image line */
unsigned char *line_buffer = NULL;
int line_count;
int linebuff_input, linebuff_output;

/* dft in/out buffers */
int
  *dft_in_r  = NULL,
  *dft_out_r = NULL,
  *dft_out_i = NULL;

/* Pixbuf rowstride and num of channels */
gint rowstride, n_channels;

/* Tree list store and treeview for stations window */
GtkListStore *stations_list_store = NULL;
GtkTreeView  *stations_treeview   = NULL;

GtkWidget
  *stations_window      = NULL, /* Stations tree list window */
  *popup_menu           = NULL, /* Popup main menu */
  *main_window          = NULL, /* Xwefax's top window */
  *scope_drawingarea    = NULL, /* Signal Scope widget */
  *spectrum_drawingarea = NULL, /* Signal Spectrum widget */
  *level_gauge          = NULL, /* The vertical level gauge in control frame */
  *text_scroller        = NULL, /* Text view scroller */
  *wefax_drawingarea    = NULL; /* Drawingarea for WEFAX image */

/* Gtk builders for some above that need to be global */
GtkBuilder
  *stations_window_builder = NULL,
  *popup_menu_builder      = NULL,
  *main_window_builder     = NULL;

/* Pixbuffer for waterfall */
GdkPixbuf *wfall_pixbuf = NULL;
guchar    *wfall_pixels = NULL;
gint
  wfall_rowstride,
  wfall_n_channels,
  wfall_width,
  wfall_height;

/* Signal scope size */
gint scope_width, scope_height;

/* Values needed to plot the level gauge */
int
  gauge_input = -1,
  gauge_level1 = 1,
  gauge_level2 = 1;

/* Text buffer for text view */
GtkTextBuffer *text_buffer = NULL;

/* Pixbuf rowstride and num of channels */
gint rowstride, n_channels;

/* Image files name  */
char image_file[MAX_FILE_NAME];

/* Buffer for reading in DSP data */
short *dsp_buffer = NULL;

/* Average value of DFT bins */
int *bin_ave = NULL;

/* What action the WEFAX decoder should enter */
int wefax_action = ACTION_STOP;

/* Fm Detector function pointer */
gboolean ( *FM_Detector ) ( unsigned char *level ) = NULL;

/* Semaphore to control async IQ data transfer */
sem_t pback_semaphore;

/*------------------------------------------------------------------------*/

