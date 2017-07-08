/*  display.c
 *
 *  Display/Rendering functions for xwefax application
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

#include "display.h"

/*------------------------------------------------------------------------*/

/* DFT_Input_Data()
 *
 * Collects and decimates signal samples for the DFT
 */
  void
DFT_Input_Data( short sample_val )
{
  static int dft_idx = 0; /* dft input buffer idx */
  static double cnt  = 0.0; /* Count of calls to this function */

  /* Summate (decimate) samples for the DFT */
  dft_in_r[dft_idx] += sample_val;

  /* Reset stride (decimation) counter */
  cnt += 1.0;
  if( cnt >= rc_data.dft_stride )
  {
	cnt -= rc_data.dft_stride;

	/* Normalize DFT input samples */
	dft_in_r[dft_idx] /= (int)rc_data.dft_stride;
	dft_idx++;

	/* Display waterfall when input buffer full */
	if( dft_idx >= DFT_INPUT_SIZE )
	{
	  Display_Waterfall();
	  dft_idx = 0;
	}

	/* Clear for next summation */
	dft_in_r[dft_idx] = 0;
  }

} /* DFT_Input_Data() */

/*------------------------------------------------------------------------*/

  int
DFT_Bin_Value( int sum_i, int sum_q, gboolean reset )
{
 /* Value of dft output "bin" */
  static int bin_val = 0;

  /* Maximum value of dft bins */
  static int bin_max = 1000, max = 0;


  /* Calculate sliding window average of max bin value */
  if( reset )
  {
	bin_max = max;
	if( !bin_max ) bin_max = 1;
	max = 0;
  }
  else
  {
	/* Calculate average signal power at each frequency (bin) */
	bin_val  = bin_val * AMPL_AVE_MUL;
	bin_val += sum_i * sum_i + sum_q * sum_q;
	bin_val /= AMPL_AVE_WIN;

	/* Record max bin value */
	if( max < bin_val ) max = bin_val;

	/* Scale bin values to 255 depending on max value */
	int ret = ( 255 * bin_val ) / bin_max;
	if( ret > 255 ) ret = 255;
	return( ret );
  }

  return( 0 );
} /* DFT_Bin_Value() */

/*------------------------------------------------------------------------*/

/* Display_Waterfall()
 *
 * Displays audio spectrum as "waterfall"
 */
  void
Display_Waterfall( void )
{
  int
	idh, idv,  /* Index to hor. and vert. position in warterfall */
	pixel_val, /* Greyscale value of pixel derived from dft o/p  */
	dft_idx,   /* Index to dft output array */
	temp;

  /* Constants needed to draw white lines in waterfall */
  static int
	white_line = 0,
	black_line = 0,
	delta_f = 0;

  static int
	white_freq = 0, /* To initialize the function */
	black_freq = 0;


  /* Pointer to current pixel */
  static guchar *pix;

  /* Constants needed to draw white lines in waterfall */
  if( (white_freq != rc_data.white_freq) ||
	  (black_freq != rc_data.black_freq) )
  {
	white_freq = rc_data.white_freq;
	black_freq = rc_data.black_freq;

	delta_f = white_freq - DFT_LOWER_FREQ;
	white_line =
	  ( delta_f * gbl_wfall_width ) /
	  ( DFT_UPPER_FREQ - DFT_LOWER_FREQ );

	delta_f = black_freq - DFT_LOWER_FREQ;
	black_line =
	  ( delta_f * gbl_wfall_width ) /
	  ( DFT_UPPER_FREQ - DFT_LOWER_FREQ );
  }

  /* Draw a vertical white line in waterfall at detector's freq. */
  pix = gbl_wfall_pixels + gbl_wfall_rowstride + gbl_wfall_n_channels;
  pix += gbl_wfall_n_channels * white_line;
  pix[0] = pix[1] = pix[2] = 0xff;
  pix = gbl_wfall_pixels + gbl_wfall_rowstride + gbl_wfall_n_channels;
  pix += gbl_wfall_n_channels * black_line;
  pix[0] = pix[1] = pix[2] = 0xff;

  /* Copy each line of waterfall to next one */
  temp = gbl_wfall_height - 2;
  for( idv = temp; idv > 0; idv-- )
  {
	pix = gbl_wfall_pixels + gbl_wfall_rowstride * idv + gbl_wfall_n_channels;

	for( idh = 0; idh < gbl_wfall_width; idh++ )
	{
	  *pix = *( pix - gbl_wfall_rowstride ); pix++;
	  *pix = *( pix - gbl_wfall_rowstride ); pix++;
	  *pix = *( pix - gbl_wfall_rowstride );
	  pix += gbl_wfall_n_channels - 2;
	}
  }

  /* Got to top left of pixbuf */
  pix = gbl_wfall_pixels;

  /* Do the DFT on input array */
  Idft( DFT_INPUT_SIZE, gbl_wfall_width );
  for( dft_idx = 0; dft_idx < gbl_wfall_width; dft_idx++ )
  {
	/* Calculate power of signal at each freq. ("bin") */
	dft_out_r[dft_idx] /= 1024;
	dft_out_i[dft_idx] /= 1024;
	pixel_val = DFT_Bin_Value(
		dft_out_r[dft_idx], dft_out_i[dft_idx], FALSE );

	/* Calculate average bin values */
	gbl_bin_ave[dft_idx] = pixel_val;

	/* Color code signal strength */
	int n;
	if( pixel_val < 85 )
	{
	  pix[0] = 0;
	  pix[1] = 0;
	  pix[2] = (guchar)(3 + pixel_val * 3);
	}
	else if( pixel_val < 170 )
	{
	  n = pixel_val - 85;
	  pix[0] = 0;
	  pix[1] = (guchar)(3 + pixel_val * 3);
	  pix[2] = (guchar)(255 - n * 3);
	}
	else
	{
	  n = pixel_val - 170;
	  pix[0] = (guchar)pixel_val;
	  pix[1] = (guchar)255;
	  pix[2] = 0;
	}

	pix += gbl_wfall_n_channels;

  } /* for( dft_idx = 0; dft_idx < DFT_SIZE2; dft_idx++ ) */

  /* Reset function */
  DFT_Bin_Value( 0, 0, TRUE );

  /* At last draw waterfall */
  gtk_widget_queue_draw( gbl_spectrum );

} /* Display_Waterfall() */

/*------------------------------------------------------------------------*/

/*  Display_Signal()
 *
 *  Updates the signal scope display
 */
  void
Display_Signal( unsigned char plot )
{
  /* Points to plot */
  static GdkPoint *points = NULL;

  static int
	height = 0,		/* Height of scope window */
	limit,			/* Limit plotting to inside of scope margin */
	points_idx = 0; /* Index to points array */

  /* Cairo context for drawing */
  static cairo_t *cr = NULL;


  /* Initialize on first call */
  if( points == NULL )
  {
	if( !mem_alloc((void **)&points,
		  sizeof(GdkPoint) * (size_t)gbl_scope_width) )
	  return;
  }

  /* Initialize on parameter change */
  if( height != gbl_scope_height )
  {
	height = gbl_scope_height;
	limit = height - SCOPE_CLEAR;

	/* Initialize Cairo */
	if( cr != NULL ) cairo_destroy( cr );
	cr = NULL;
	cr = gdk_cairo_create( gbl_scope->window );
  }

  /* Save values to be plotted */
  plot = (unsigned char)( ((int)plot * limit ) / 255 );
  points[points_idx].y = limit - (gint)plot;
  if( points[points_idx].y < SCOPE_CLEAR )
	points[points_idx].y = SCOPE_CLEAR;
  points[points_idx].x = points_idx;

  /* Recycle buffer idx when full and plot */
  if( ++points_idx == gbl_scope_width )
  {
	int idx;

	/* Draw scope backgrounds */
	cairo_set_source_rgb( cr, SCOPE_BACKGND );
	cairo_rectangle(
		cr, 0.0, 0.0,
		(double)gbl_scope_width,
		(double)gbl_scope_height );
	cairo_fill( cr );

	/* Plot signal graph */
	cairo_set_source_rgb( cr, SCOPE_FOREGND );

	cairo_move_to( cr,
		(double)points[0].x,
		(double)points[0].y );
	for( idx = 1; idx < points_idx; idx++ )
	  cairo_line_to( cr,
		  (double)points[idx].x,
		  (double)points[idx].y );

	/* Stroke paths */
	cairo_stroke( cr );

	points_idx = 0;
  } /* if( ++points_idx == wfall_pixbuf.width ) */

} /* Display_Signal( void ) */

/*------------------------------------------------------------------------*/

/* Set_Indicators()
 *
 * Sets the indicator icons ("LEDs") in the Control frame
 */
  void
Set_Indicators( int flag )
{
  /* Container positions of indicator icons */
  guint left, right, top, bottom;
  static gboolean first_call = TRUE;

  /* The control widgets container */
  static GtkTable *control_table = NULL;

  /* The images (icons) of control frame */
  static GtkWidget
	*start_icon  = NULL,
	*sync_icon   = NULL,
	*decode_icon = NULL,
	*save_icon   = NULL;

  GtkWidget **icon = NULL;
  gchar		 *name = NULL;

  /* Get the control widgets table */
  if( first_call )
  {
	control_table = GTK_TABLE(
		lookup_widget(main_window, "control_table") );

	start_icon  = lookup_widget( main_window, "start_icon" );
	sync_icon   = lookup_widget( main_window, "sync_icon" );
	decode_icon = lookup_widget( main_window, "decode_icon" );
	save_icon   = lookup_widget( main_window, "save_icon" );

	first_call = FALSE;
  } /* if( first_call ) */

  /* Act according to icon select flag */
  switch( flag )
  {
	case ICON_START_YES:
	  icon = &start_icon;
	  name = "gtk-yes";
	  break;

	case ICON_SYNC_YES:
	  icon = &sync_icon;
	  name = "gtk-yes";
	  break;

	case ICON_DECODE_YES:
	  icon = &decode_icon;
	  name = "gtk-yes";
	  break;

	case ICON_SAVE_YES:
	  icon = &save_icon;
	  name = "gtk-yes";
	  break;

	case ICON_START_NO:
	  icon = &start_icon;
	  name = "gtk-no";
	  break;

	case ICON_SYNC_NO:
	  icon = &sync_icon;
	  name = "gtk-no";
	  break;

	case ICON_DECODE_NO:
	  icon = &decode_icon;
	  name = "gtk-no";
	  break;

	case ICON_SAVE_NO:
	  icon = &save_icon;
	  name = "gtk-no";
	  break;

	case ICON_START_SKIP:
	  icon = &start_icon;
	  name = "gtk-stop";
	  break;

	case ICON_SYNC_SKIP:
	  icon = &sync_icon;
	  name = "gtk-stop";
	  break;

	case ICON_DECODE_SKIP:
	  icon = &decode_icon;
	  name = "gtk-stop";
	  break;

	case ICON_SAVE_SKIP:
	  icon = &save_icon;
	  name = "gtk-stop";
	  break;

	case ICON_START_APPLY:
	  icon = &start_icon;
	  name = "gtk-apply";
	  break;

	case ICON_SYNC_APPLY:
	  icon = &sync_icon;
	  name = "gtk-apply";
	  break;

	case ICON_DECODE_APPLY:
	  icon = &decode_icon;
	  name = "gtk-apply";
	  break;

	case ICON_SAVE_APPLY:
	  icon = &save_icon;
	  name = "gtk-apply";
	  break;

	default: return;

  } /* switch( flag ) */

  /* Get icon's position */
  gtk_container_child_get(
	  GTK_CONTAINER(control_table),
	  *icon,
	  "left-attach",   &left,
	  "right-attach",  &right,
	  "top-attach",    &top,
	  "bottom-attach", &bottom,
	  NULL );

  /* Remove previous icon */
  gtk_container_remove( GTK_CONTAINER(control_table), *icon );

  /* Make new icon */
  *icon = gtk_image_new_from_stock( name, GTK_ICON_SIZE_BUTTON );

  /* Attach a new icon */
  gtk_widget_show( *icon );
  gtk_container_add_with_properties(
	  GTK_CONTAINER(control_table), *icon,
	  "left-attach",   left,
	  "right-attach",  right,
	  "top-attach",    top,
	  "bottom-attach", bottom,
	  NULL );

} /* Set_Indicators() */

/*------------------------------------------------------------------------*/

/* Set_Menu_Items()
 *
 * Sets the appropriate menu items to reflect
 * the settings in the configuration file
 */
  void
Set_Menu_Items( void )
{
  GtkCheckMenuItem *item;

  int rpm[ NUM_RPM ] = { RPM60, RPM90, RPM100, RPM120, RPM180, RPM240 };
  int pix[ NUM_PIX ] = { PIX600, PIX1200 };
  int ioc[ NUM_IOC ] = { IOC288, IOC576 };
  int phl[ NUM_PHL ] = { PHL10, PHL20, PHL40, PHL60 };
  int ime[ NUM_IME ] = { IME0, IME1, IME2 };

  char name[8];
  int idx;

  /* Find current RPM value */
  for( idx = 0; idx < NUM_RPM; idx++ )
	if( rpm[ idx ] == (int)rc_data.lines_per_min )
	  break;
  if( idx == NUM_RPM ) return;

  /* Set active RPM menu item */
  snprintf( name, sizeof(name), "rpm%d", rpm[idx] );
  name[7] = '\0';
  item = GTK_CHECK_MENU_ITEM( lookup_widget(gbl_popup_menu, name) );
  gtk_check_menu_item_set_active( item, TRUE );

  /* Find current resolution value */
  for( idx = 0; idx < NUM_PIX; idx++ )
	if( pix[ idx ] == rc_data.pixels_per_line )
	  break;
  if( idx == NUM_PIX ) return;

  /* Set active resolution menu item */
  snprintf( name, sizeof(name), "pix%d", pix[idx] );
  name[7] = '\0';
  item = GTK_CHECK_MENU_ITEM( lookup_widget(gbl_popup_menu, name) );
  gtk_check_menu_item_set_active( item, TRUE );

  /* Find current IOC value */
  for( idx = 0; idx < NUM_IOC; idx++ )
	if( ioc[ idx ] == rc_data.ioc_value )
	  break;
  if( idx == NUM_IOC ) return;

  /* Set active IOC menu item */
  snprintf( name, sizeof(name), "ioc%d", ioc[idx] );
  name[7] = '\0';
  item = GTK_CHECK_MENU_ITEM( lookup_widget(gbl_popup_menu, name) );
  gtk_check_menu_item_set_active( item, TRUE );

  /* Find current phasing lines value */
  for( idx = 0; idx < NUM_PHL; idx++ )
	if( phl[ idx ] == rc_data.phasing_lines )
	  break;
  if( idx == NUM_PHL ) return;

  /* Set active phasing lines menu item */
  snprintf( name, sizeof(name), "phl%d", phl[idx] );
  name[7] = '\0';
  item = GTK_CHECK_MENU_ITEM( lookup_widget(gbl_popup_menu, name) );
  gtk_check_menu_item_set_active( item, TRUE );

  /* Find current image enhancement setting */
  for( idx = 0; idx < NUM_IME; idx++ )
	if( ime[ idx ] == rc_data.image_enhance )
	  break;
  if( idx == NUM_IME ) return;

  /* Set active image enhancement menu item */
  snprintf( name, sizeof(name), "ime%d", ime[idx] );
  name[7] = '\0';
  item = GTK_CHECK_MENU_ITEM( lookup_widget(gbl_popup_menu, name) );
  gtk_check_menu_item_set_active( item, TRUE );

} /* Set_Menu_Items() */

/*------------------------------------------------------------------------*/

/*  Normalize()
 *
 *  Does histogram (linear) normalization of a WEFAX line
 */
  void
Normalize( unsigned char *line_buffer, int line_len )
{
  int
	hist[256],  /* Intensity histogram of pgm image file  */
	blk_cutoff, /* Count of pixels for black cutoff value */
	wht_cutoff, /* Count of pixels for white cutoff value */
	pixel_val,  /* Used for calculating normalized pixels */
	pixel_cnt,  /* Total pixels counter for cut-off point */
	idx;        /* Index for loops etc */

  int
	black_val,  /* Black cut-off pixel intensity value */
	white_val,  /* White cut-off pixel intensity value */
	val_range;  /* Range of intensity values in image  */

  if( line_len <= 0 )
  {
	Show_Message( _("Image line length zero\n"\
		  "Normalization not performed"), "red" );
	return;
  }

  /* Clear histogram */
  bzero( (void *)hist, sizeof(hist) );

  /* Build image intensity histogram */
  for( idx = 0; idx < line_len; idx++ )
	hist[ line_buffer[idx] ] += 1;

  /* Determine black/white cut-off counts */
  blk_cutoff = (line_len * BLACK_CUT_OFF) / 100;
  wht_cutoff = (line_len * WHITE_CUT_OFF) / 100;

  /* Find black cut-off intensity value */
  pixel_cnt = 0;
  for( black_val = 0; black_val <= 255; black_val++ )
  {
	pixel_cnt += hist[ black_val ];
	if( pixel_cnt > blk_cutoff ) break;
  }

  /* Find white cut-off intensity value */
  pixel_cnt = 0;
  for( white_val = 255; white_val >= 0; white_val-- )
  {
	pixel_cnt += hist[ white_val ];
	if( pixel_cnt > wht_cutoff ) break;
  }

  /* Rescale pixels in image for full intensity range */
  val_range = white_val - black_val;
  if( val_range <= 0 ) return;

  /* Perform histogram normalization on images */
  for( pixel_cnt = 0; pixel_cnt < line_len; pixel_cnt++ )
  {
	pixel_val = line_buffer[ pixel_cnt ];
	pixel_val = ( (pixel_val - black_val) * 255 ) / val_range;

	pixel_val = ( pixel_val < 0 ? 0 : pixel_val );
	pixel_val = ( pixel_val > 255 ? 255 : pixel_val );
	line_buffer[ pixel_cnt ] = (unsigned char)pixel_val;
  }

} /* End of Normalize() */

/*------------------------------------------------------------------------*/

/* Spectrum_Size_Allocate()
 *
 * Handles the size_allocate callback on spectrum drawingarea
 */
  void
Spectrum_Size_Allocate( GdkRectangle *allocation )
{
  int idx;

  /* Destroy existing pixbuff */
  if( gbl_wfall_pixbuf != NULL )
  {
	g_object_unref( G_OBJECT(gbl_wfall_pixbuf) );
	gbl_wfall_pixbuf = NULL;
  }

  /* Create waterfall pixbuf */
  gbl_wfall_pixbuf = gdk_pixbuf_new(
	  GDK_COLORSPACE_RGB, FALSE, 8,
	  allocation->width, allocation->height );
  if( gbl_wfall_pixbuf == NULL ) return;

  gbl_wfall_pixels = gdk_pixbuf_get_pixels( gbl_wfall_pixbuf );
  gbl_wfall_width  = gdk_pixbuf_get_width ( gbl_wfall_pixbuf );
  gbl_wfall_height = gdk_pixbuf_get_height( gbl_wfall_pixbuf );
  gbl_wfall_rowstride  = gdk_pixbuf_get_rowstride( gbl_wfall_pixbuf );
  gbl_wfall_n_channels = gdk_pixbuf_get_n_channels( gbl_wfall_pixbuf );
  gdk_pixbuf_fill( gbl_wfall_pixbuf, 0 );

  /* Calculate dft stride to put upper freq at end of DFT
   * display and allow a freq range max/min ratio of 2:1 */
  idx = DFT_INPUT_SIZE / gbl_wfall_width / DFT_FREQ_MULTP;
  rc_data.dft_stride =
	(double)rc_data.dsp_rate / (double)DFT_UPPER_FREQ / (double)idx;

  /* Allocate average bin value buffer */
  if( !mem_realloc((void **)&gbl_bin_ave,
		(size_t)gbl_wfall_width * sizeof(int)) )
	return;

  /* Initialize dft and config */
  for( idx = 0; idx < gbl_wfall_width; idx++ )
	gbl_bin_ave[idx] = 0;
  Idft_Init( DFT_INPUT_SIZE, gbl_wfall_width );

} /* Spectrum_Size_Allocate() */

/*------------------------------------------------------------------------*/

/* Set_Sync_Slant()
 *
 * Sets the value of parameters used to deslant WEFAX image
 */
  void
Set_Sync_Slant( double sync_slant )
{
  /* Add Perseus ADC Rate correction */
  if( rc_data.tcvr_type == PERSEUS )
	sync_slant += rc_data.perseus_rate_correction;

  /* Pixels/1000 lines sync slant correction */
  rc_data.sync_slant = sync_slant / 1000.0;

  /* Length (duration) of an image pixel in DSP samples */
  double temp = rc_data.lines_per_min / 60.0; /* lines/sec */
  if( temp != 0.0 )
	temp = (double)rc_data.dsp_rate / temp; /* samples/line  */

  /* Add sync slant correction to pixel length */
  rc_data.pixel_len = temp;
  temp = (double)rc_data.pixels_per_line + rc_data.sync_slant;

  /* Samples/pixel as a float */
  if( temp != 0.0 )
	rc_data.pixel_len = rc_data.pixel_len / temp;

} /* Set_Sync_Slant() */

/*------------------------------------------------------------------------*/

/* Display_Level_Gauge()
 *
 * Displays a level gauge for signal
 * strength | stop train | start train
 */
  void
Display_Level_Gauge( int input, int level1, int level2 )
{
  cairo_t *cr = NULL;
  GtkWidget *level_gauge = NULL;

  /* Width and height of gauge widget */
  int width = 0, height = 0;

  /* Count of calls to function */
  static int cnt = 0;

  /* Width and height of gauge bar */
  double bar_width, bar_height;

  int level;

  /* Display gauge only every GAUGE_COUNT calls */
  if( (cnt++ < GAUGE_COUNT) && (input >= 0) ) return;
  cnt = 0;

  /* It happens if the call is
   * from expose event callback */
  if( input < 0 ) input = 0;

  /* Create level gauge */
  level_gauge = lookup_widget( main_window, "control_drawingarea" );

  /* Create a Cairo context */
  height = level_gauge->allocation.height;
  width  = level_gauge->allocation.width;
  bar_width  = (double)width  - 1.0;
  bar_height = (double)height - 1.0;
  cr = gdk_cairo_create( level_gauge->window );

  /* Create red bar graph of input level */
  level = input;
  if( level > level1 ) level = level1;
  cairo_set_source_rgb( cr, GAUGE_RED );
  cairo_rectangle( cr,
	  1.0, bar_height - (double)level,
	  bar_width,
	  (double)level );
  cairo_fill( cr );

  /* Create yellow bar graph of input level */
  level = input;
  if( level > level1 )
  {
	if( level > level2 ) level = level2;
	cairo_set_source_rgb( cr, GAUGE_YELLOW );
	cairo_rectangle( cr,
		1.0, bar_height - (double)level,
		bar_width,
		(double)level - (double)level1 );
	cairo_fill( cr );
  }

  /* Create green bar graph of input level */
  level = input;
  if( level > level2 )
  {
	if( level > bar_height ) level = bar_height;
	cairo_set_source_rgb( cr, GAUGE_GREEN );
	cairo_rectangle( cr,
		1.0, bar_height - (double)level,
		bar_width,
		(double)level - (double)level2 );
	cairo_fill( cr );
  }

  /* Clear rest of bar graph */
  cairo_set_source_rgb( cr, GAUGE_WHITE );
  cairo_rectangle( cr,
	  1.0, 1.0,
	  bar_width,
	  bar_height - (double)level );
  cairo_fill( cr );

  /* Create bar graph outline of sync level */
  cairo_set_source_rgb( cr, GAUGE_GREY );
  cairo_rectangle( cr,
	  0.0, 0.0,
	  (double)width,
	  (double)height );
  cairo_stroke( cr );

  if( cr ) cairo_destroy( cr );

} /* Display_Level_Gauge() */

/*------------------------------------------------------------------------*/

