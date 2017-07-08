/*  shared.h
 *
 *  Shared variables for xwefax application
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

#ifndef SHARED_H
#define SHARED_H	1

#include "common.h"
#include "sound.h"

/* Runtime config data */
extern rc_data_t rc_data;

/* Pixel buffer */
extern guchar *gbl_pixel_buf;

/* Pixel buffer for display */
extern GdkPixbuf *gbl_wefax_pixbuf;

/* Buffer for pixels of one image line */
extern unsigned char *gbl_line_buffer;
extern int gbl_line_count;
extern int gbl_linebuff_input, gbl_linebuff_output;

/* dft in/out buffers */
extern int
  *dft_in_r,
  *dft_out_r,
  *dft_out_i;

/* Pixbuf rowstride and num of channels */
extern gint
  gbl_rowstride,
  gbl_n_channels;

/* Tree list store */
extern GtkListStore *stations_list_store;
extern GtkTreeView *stations_treeview;

extern GtkWidget
  *stations_window,			/* Stations list window */
  *gbl_popup_menu,			/* Popup main menu */
  *main_window,				/* xwefax's top window */
  *gbl_scope,				/* Signal scope widget */
  *gbl_spectrum,			/* Signal spectrum widget */
  *gbl_text_scroller,		/* Text view scroller */
  *gbl_wefax_drawingarea;   /* Drawingarea to display WEFAX image */

/* Pixbuffer for waterfall */
extern GdkPixbuf *gbl_wfall_pixbuf;
extern guchar    *gbl_wfall_pixels;
extern gint
  gbl_wfall_rowstride,
  gbl_wfall_n_channels,
  gbl_wfall_width,
  gbl_wfall_height;

/* Signal scope size */
extern gint
  gbl_scope_width,
  gbl_scope_height;

/* Text buffer for text view */
extern GtkTextBuffer *gbl_text_buffer;

/* Pixbuf rowstride and num of channels */
extern gint
  gbl_rowstride,
  gbl_n_channels;

/* Image files name  */
extern char gbl_image_file[MAX_FILE_NAME];

/* Buffer for reading in DSP data */
extern short *gbl_dsp_buffer;

/* Average value of DFT bins */
extern int *gbl_bin_ave;

/* What action the WEFAX decoder should enter */
extern int wefax_action;

/* Fm Detector function pointer */
gboolean ( *FM_Detector ) ( unsigned char *level );

/* Semaphore to control async IQ data transfer */
extern sem_t pback_semaphore;

#define SCOPE_BACKGND	0.0, 0.3, 0.0
#define PIXBUF_BACKGND	0xb0b0b0ff

#endif

