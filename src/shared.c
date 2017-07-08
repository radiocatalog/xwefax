/*  shared.c
 *
 *  Shared global variables for xwefax application
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

#include "shared.h"

/* Runtime config data */
rc_data_t rc_data;

/* Pixel buffer */
guchar *gbl_pixel_buf;

/* Pixel buffer for display */
GdkPixbuf *gbl_wefax_pixbuf = NULL;

/* Buffer for pixels of one image line */
unsigned char *gbl_line_buffer = NULL;
int gbl_line_count;
int gbl_linebuff_input, gbl_linebuff_output;

/* dft in/out buffers */
int
  *dft_in_r  = NULL,
  *dft_out_r = NULL,
  *dft_out_i = NULL;

/* Pixbuf rowstride and num of channels */
gint gbl_rowstride, gbl_n_channels;

/* Tree list store and treeview for stations window */
GtkListStore *stations_list_store = NULL;
GtkTreeView  *stations_treeview   = NULL;

GtkWidget
  *stations_window = NULL,	/* Stations tree list window */
  *gbl_popup_menu  = NULL,	/* Popup main menu */
  *main_window,				/* xwefax's top window */
  *gbl_scope,				/* Signal scope widget */
  *gbl_spectrum,			/* Signal Spectrum widget */
  *gbl_text_scroller,		/* Text view scroller  */
  *gbl_wefax_drawingarea;	/* Drawingarea for WEFAX image */

/* Pixbuffer for waterfall */
GdkPixbuf *gbl_wfall_pixbuf = NULL;
guchar    *gbl_wfall_pixels = NULL;
gint
  gbl_wfall_rowstride,
  gbl_wfall_n_channels,
  gbl_wfall_width,
  gbl_wfall_height;

/* Signal scope size */
gint gbl_scope_width, gbl_scope_height;

/* Text buffer for text view */
GtkTextBuffer *gbl_text_buffer = NULL;

/* Pixbuf rowstride and num of channels */
gint gbl_rowstride, gbl_n_channels;

/* Image files name  */
char gbl_image_file[MAX_FILE_NAME];

/* Buffer for reading in DSP data */
short *gbl_dsp_buffer = NULL;

/* Average value of DFT bins */
int *gbl_bin_ave = NULL;

/* What action the WEFAX decoder should enter */
int wefax_action = ACTION_STOP;

/* Fm Detector function pointer */
gboolean ( *FM_Detector ) ( unsigned char *level ) = NULL;

/* Semaphore to control async IQ data transfer */
sem_t pback_semaphore;

/*------------------------------------------------------------------------*/

