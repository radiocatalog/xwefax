/*  utils.c
 *
 *  Utility functions of xwefax application
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

#include "utils.h"

/*------------------------------------------------------------------*/

/*  Load_Line()
 *
 *  Loads a line from a file, aborts on failure. Lines beginning
 *  with a '#' are ignored as comments. At the end of file EOF is
 *  returned. Lines assumed maximum 80 characters long.
 */

  int
Load_Line( char *buff, FILE *pfile, char *mesg )
{
  int
	num_chr, /* Number of characters read, excluding lf/cr */
	chr;     /* Character read by getc() */
  char error_mesg[MESG_SIZE];

  /* Prepare error message */
  snprintf( error_mesg, sizeof(error_mesg),
	  _("Error reading %s\n"\
		"Premature EOF (End Of File)"), mesg );

  /* Clear buffer at start */
  buff[0] = '\0';
  num_chr = 0;

  /* Get next character, return if chr = EOF */
  chr = fgetc( pfile );
  if( chr == EOF ) return( EOF );

  /* Ignore commented lines, lines starting
   * with white spaces and newline/carriage ret */
  while(
	  (chr == '#') ||
	  (chr == ' ') ||
	  (chr == HT ) ||
	  (chr == CR ) ||
	  (chr == LF ) )
  {
	/* Go to the end of line (look for LF or CR) */
	while( (chr != CR) && (chr != LF) )
	{
	  /* Get next character, return error if chr = EOF */
	  chr = fgetc( pfile );
	  if( chr == EOF )
	  {
		Show_Message( error_mesg, "red" );
		fprintf( stderr, "xwefax: %s\n", error_mesg );
		fclose( pfile );
		Error_Dialog( error_mesg, QUIT );
		return( ERROR );
	  }
	}

	/* Dump any CR/LF remaining */
	while( (chr == CR) || (chr == LF) )
	{
	  /* Get next character, return error if chr = EOF */
	  chr = fgetc( pfile );
	  if( chr == EOF )
	  {
		Show_Message( error_mesg, "red" );
		fprintf( stderr, "xwefax: %s\n", error_mesg );
		fclose( pfile );
		Error_Dialog( error_mesg, QUIT );
		return( ERROR );
	  }
	}

  } /* End of while( (chr == '#') || ... */

  /* Continue reading characters from file till
   * number of characters = 80 or EOF or CR/LF */
  while( num_chr < LINE_BUFF_LEN )
  {
	/* If LF/CR reached before filling buffer, return line */
	if( (chr == LF) || (chr == CR) )
	{
	  /* Get next character */
	  chr = fgetc( pfile );
	  if( chr == EOF )
	  {
		/* Terminate buffer as a string if chr = EOF */
		buff[num_chr] = '\0';
		return( EOF );
	  }

	  /* Restore char in not EOF */
	  ungetc( chr, pfile );
	  break;
	}

	/* Enter new character to line buffer */
	buff[num_chr++] = (char)chr;

	/* Get next character */
	chr = fgetc( pfile );
	if( chr == EOF )
	{
	  /* Terminate buffer as a string if chr = EOF */
	  buff[num_chr] = '\0';
	  return( EOF );
	}

	/* Abort if end of line not reached at 80 char. */
	if( (num_chr == LINE_BUFF_LEN) && (chr != LF) && (chr != CR) )
	{
	  /* Terminate buffer as a string */
	  buff[num_chr] = '\0';
	  snprintf( error_mesg, sizeof(error_mesg),
		  _("Error reading %s\n"\
			"Line longer than %d characters"), mesg, LINE_BUFF_LEN );
	  Show_Message( error_mesg, "red" );
	  fprintf( stderr, "xwefax: %s\n%s\n", error_mesg, buff );
	  fclose( pfile );
	  Error_Dialog( error_mesg, QUIT );
	  return( ERROR );
	}

  } /* End of while( num_chr < max_chr ) */

  /* Terminate buffer as a string */
  buff[num_chr] = '\0';

  return( SUCCESS );
} /* End of Load_Line() */

/*------------------------------------------------------------------*/

/*  Load_Config()
 *
 *  Loads the xwefaxrc configuration file
 */

  gboolean
Load_Config( gpointer data )
{
  int idx;

  char
	rc_fpath[64], /* File path to xwefaxrc */
	line[81];     /* Buffer for Load_Line  */

  /* Config file pointer */
  FILE *xwefaxrc;

  /* ALSA channel names and values */
  char *chan_name[] =
  {
	"FRONT_LEFT",
	"FRONT_RIGHT",
	"REAR_LEFT",
	"REAR_RIGHT",
	"SIDE_LEFT",
	"SIDE_RIGHT",
	"MONO"
  };

  int chan_number[] =
  {
	SND_MIXER_SCHN_FRONT_LEFT,
	SND_MIXER_SCHN_FRONT_RIGHT,
	SND_MIXER_SCHN_REAR_LEFT,
	SND_MIXER_SCHN_REAR_RIGHT,
	SND_MIXER_SCHN_SIDE_LEFT,
	SND_MIXER_SCHN_SIDE_RIGHT,
	SND_MIXER_SCHN_MONO
  };
  int num_chn = 7;

  /* Setup file path to xwefaxrc */
  snprintf( rc_fpath, sizeof(rc_fpath),
	  "%s/xwefax/xwefaxrc", getenv("HOME") );

  /* Open xwefaxrc file */
  xwefaxrc = fopen( rc_fpath, "r" );
  if( xwefaxrc == NULL )
  {
	perror( rc_fpath );
	Show_Message(
		_("Failed to open xwefaxrc file\n"\
		  "Quit xwefax and correct"), "red" );
	Error_Dialog(
		_("Failed to open xwefaxrc file\n"\
		  "Quit xwefax and correct"), QUIT );
	return( FALSE );
  }

  /*** Read Sound Card configuration data ***/

  /* Read card name, abort if EOF */
  if( Load_Line(line, xwefaxrc, _("Sound Card Name")) != SUCCESS )
	return( FALSE );
  Strlcpy( rc_data.snd_card, line, sizeof(rc_data.snd_card) );

  /* Read pcm device name, abort if EOF */
  if( Load_Line(line, xwefaxrc,	_("PCM Device Name")) != SUCCESS )
	return( FALSE );
  Strlcpy( rc_data.pcm_dev, line, sizeof(rc_data.pcm_dev) );

  /* Read DSP rate Samples/sec, abort if EOF */
  if( Load_Line(line, xwefaxrc,	_("DSP Rate") ) != SUCCESS )
	return( FALSE );
  rc_data.dsp_rate = atoi( line );

  /* Read ALSA "channel", abort if EOF */
  if( Load_Line(line, xwefaxrc,	_("ALSA Channel") ) != SUCCESS )
	return( FALSE );
  for( idx = 0; idx < num_chn; idx++ )
	if( strcmp(chan_name[idx], line) == 0 )
	  break;
  if( idx == num_chn )
  {
	fclose( xwefaxrc );
	Show_Message(
		_("Invalid ALSA channel name\n"\
		  "Quit and correct xwefaxrc"), "red" );
	Error_Dialog(
		_("Invalid ALSA channel name\n"\
		  "Quit and correct xwefaxrc"), QUIT);
	return( FALSE );
  }
  rc_data.channel = chan_number[idx];

  /* Set right or left channel buffer index */
  if( strstr(line, _("LEFT")) || strstr(line, _("MONO")) )
	rc_data.use_chn = 0;
  else
	rc_data.use_chn = 1;

  /* Set number of channels */
  if( strstr(line, _("MONO")) )
	rc_data.num_chn = 1;
  else
	rc_data.num_chn = 2;

  /* Read capture source, abort if EOF */
  if( Load_Line(line, xwefaxrc,	_("Capture Source")) != SUCCESS )
	return( FALSE );
  Strlcpy( rc_data.cap_src, line, sizeof(rc_data.cap_src) );

  /* Read Capture volume control, abort if EOF */
  if( Load_Line(line, xwefaxrc,	_("Capture Volume Control")) != SUCCESS )
	return( FALSE );
  Strlcpy( rc_data.cap_vol, line, sizeof(rc_data.cap_vol) );

  /* Read Capture volume, abort if EOF */
  if( Load_Line(line, xwefaxrc,	_("Capture Volume")) != SUCCESS )
	return( FALSE );
  rc_data.cap_lev = atoi( line );

  /*** End of Sound Card configuration data ***/

  /*** Runtime configuration data ***/

  /* Use the Perseus SDR Receiver */
  if( Load_Line(line, xwefaxrc, _("Perseus SDR flag")) != SUCCESS )
	return( FALSE );
  if( strcmp(line, "PERSEUS") == 0 )
  {
	rc_data.tcvr_type = PERSEUS;
	rc_data.dsp_rate  = PERSEUS_SAMPLE_RATE;
  }	
  else if( strcmp(line, "RADIO") == 0 )
	rc_data.tcvr_type = RADIO;
  else
  {
	rc_data.tcvr_type = NONE;
	fclose( xwefaxrc );
	Error_Dialog(
		_("Error reading Perseus SDR flag\n"\
		  "Quit and correct xwefaxrc"), QUIT );
	return( FALSE );
  }

  /* Read the Perseus XO frequency correction factor */
  if( Load_Line(line, xwefaxrc, _("Perseus Frequency Correction")) != SUCCESS )
	return( FALSE );
  rc_data.perseus_freq_correction = atof( line );

  /* Read the Perseus adc rate correction factor */
  if( Load_Line(line, xwefaxrc, _("Perseus ADC Rate Correction")) != SUCCESS )
	return( FALSE );
  rc_data.perseus_rate_correction = atoi( line );

  /* Read CAT enable flag, abort if EOF */
  if( Load_Line(line, xwefaxrc, _("Transceiver Type")) != SUCCESS )
	return( FALSE );
  if( rc_data.tcvr_type == RADIO )
  {
	if( strcmp(line, "FT847") == 0 )
	  rc_data.tcvr_type = FT847;
	else if( strcmp(line, "FT857") == 0 )
	  rc_data.tcvr_type = FT857;
	else if( strcmp(line, "K2") == 0 )
	  rc_data.tcvr_type = K2;
	else if( strcmp(line, "K3") == 0 )
	  rc_data.tcvr_type = K3;
	else if( strcmp(line, "NONE") == 0 )
	  rc_data.tcvr_type = NONE;
	else
	{
	  rc_data.tcvr_type = NONE;
	  fclose( xwefaxrc );
	  Error_Dialog(
		  _("Error reading Transceiver type\n"\
			"Quit and correct xwefaxrc"), QUIT );
	  return( FALSE );
	}

  } /* if( rc_data.tcvr_type == RADIO ) */

  /* Enable Transceiver CAT */
  if( (rc_data.tcvr_type == NONE) ||
	  (rc_data.tcvr_type == PERSEUS) )
	ClearFlag( ENABLE_CAT );
  else
	SetFlag( ENABLE_CAT );

  /* Read CAT serial port device, abort if EOF */
  if( Load_Line(line, xwefaxrc, _("CAT Serial Port")) != SUCCESS )
	return( FALSE );
  Strlcpy( rc_data.cat_serial, line, sizeof(rc_data.cat_serial) );

  /* Read default main window height, abort if EOF */
  if( Load_Line(line, xwefaxrc,	_("Main Window Height")) != SUCCESS )
	return( FALSE );
  rc_data.window_height = atoi( line );

  /* Read white tone frequency */
  if( Load_Line(line, xwefaxrc,	_("White Tone Frequency")) != SUCCESS )
	return( FALSE );
  rc_data.white_freq = atoi( line );
  if( (rc_data.white_freq < 2100) || (rc_data.white_freq > 2500) )
  {
	fclose( xwefaxrc );
	Show_Message(
		_("Error reading White Tone Freq\n"\
		  "Quit and correct xwefaxrc"), "red" );
	Error_Dialog(
		_("Error reading White Tone Freq\n"\
		  "Quit and correct xwefaxrc"), QUIT );
	return( FALSE );
  }

  /* Read black tone frequency */
  if( Load_Line(line, xwefaxrc,	_("Black Tone Frequency")) != SUCCESS )
	return( FALSE );
  rc_data.black_freq = atoi( line );
  if( (rc_data.black_freq < 1300) || (rc_data.black_freq > 1700) )
  {
	fclose( xwefaxrc );
	Show_Message(
		_("Error reading Black Tone Freq\n"\
		  "Quit and correct xwefaxrc"), "red" );
	Error_Dialog(
		_("Error reading Black Tone Freq\n"\
		  "Quit and correct xwefaxrc"), QUIT );
	return( FALSE );
  }

  /* Read default number of lines to decode, abort if EOF */
  if( Load_Line(line, xwefaxrc,	_("Maximum Lines to Decode")) != SUCCESS )
	return( FALSE );
  rc_data.image_lines = atoi( line );
  if( (rc_data.image_lines < 120) || (rc_data.image_lines > 3000) )
  {
	fclose( xwefaxrc );
	Show_Message(
		_("Specified Maximum Number of Lines"\
		  "to Decode seems unreasonable\n"\
		  "Quit and correct xwefaxrc"), "red" );
	Error_Dialog(
		_("Specified Maximum Number of Lines"\
		  "to Decode seems unreasonable\n"\
		  "Quit and correct xwefaxrc"), QUIT );
	return( FALSE );
  }

  /* Read default lines per minute, abort if EOF */
  if( Load_Line(line, xwefaxrc,	_("Lines per Minute")) != SUCCESS )
	return( FALSE );
  rc_data.lines_per_min = atof( line );
  if( (rc_data.lines_per_min < 60.0) || (rc_data.lines_per_min > 1000.0) )
  {
	fclose( xwefaxrc );
	Show_Message(
		_("Error reading RPM (Lines/Minute)\n"\
		  "Quit and correct xwefaxrc"), "red" );
	Error_Dialog(
		_("Error reading RPM (Lines/Minute)\n"\
		  "Quit and correct xwefaxrc"), QUIT );
	return( FALSE );
  }

  /* Read default image resolution in pixels, abort if EOF */
  if( Load_Line(line, xwefaxrc,	_("Wefax Image Resolution")) != SUCCESS )
	return( FALSE );
  rc_data.pixels_per_line = atoi( line );
  if( (rc_data.pixels_per_line < 120) || (rc_data.pixels_per_line > 1200) )
  {
	fclose( xwefaxrc );
	Show_Message(
		_("Error reading Image Resolution\n"\
		  "(Pixels per Image Line)\n"\
		  "Quit and correct xwefaxrc"), "red" );
	Error_Dialog(
		_("Error reading Image Resolution\n"\
		  "(Pixels per Image Line)\n"\
		  "Quit and correct xwefaxrc"), QUIT );
	return( FALSE );
  }

  /* Read default IOC value, abort if EOF */
  if( Load_Line(line, xwefaxrc, _("Default IOC Value")) != SUCCESS )
	return( FALSE );
  rc_data.ioc_value = atoi( line );
  if( (rc_data.ioc_value != 288) && (rc_data.ioc_value != 576) )
  {
	fclose( xwefaxrc );
	Show_Message(
		_("Error reading IOC Value\n"\
		  "Quit and correct xwefaxrc"), "red" );
	Error_Dialog(
		_("Error reading IOC Value\n"\
		  "Quit and correct xwefaxrc"), QUIT );
	return( FALSE );
  }

  /* Set the default start tone */
  if( rc_data.ioc_value == IOC576 )
	rc_data.start_tone = IOC576_START_TONE;
  else
	rc_data.start_tone = IOC288_START_TONE;

  /* Read default number of phasing lines, abort if EOF */
  if( Load_Line(line, xwefaxrc,	_("Number of Phasing Lines")) != SUCCESS )
	return( FALSE );
  rc_data.phasing_lines = atoi( line );
  if( (rc_data.phasing_lines < 10) || (rc_data.phasing_lines > 60) )
  {
	fclose( xwefaxrc );
	Show_Message(
		_("Error reading Default\n"
		  "Number of Phasing Lines\n"\
		  "Quit and correct xwefaxrc"), "red" );
	Error_Dialog(
		_("Error reading Default\n"
		  "Number of Phasing Lines\n"\
		  "Quit and correct xwefaxrc"), QUIT );
	return( FALSE );
  }

  /* Read default image enhancement algorithm, abort if EOF */
  if( Load_Line(line, xwefaxrc,	_("Wefax Image Enhancement")) != SUCCESS )
	return( FALSE );
  rc_data.image_enhance = atoi( line );

  /* Form the xwefax home directory */
  snprintf( rc_data.xwefax_dir,
	  sizeof(rc_data.xwefax_dir),
	  "%s/xwefax/", getenv("HOME") );

   /* Form the xwefax stations file name */
  snprintf( rc_data.stations_file,
	  sizeof(rc_data.stations_file),
	  "%s/xwefax/stations", getenv("HOME") );

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );

  /* Initialize xwefax runtime config */
  Configure();
  Set_Menu_Items();
  fclose( xwefaxrc );
  FM_Detector = FM_Detect_Zero_Crossing;
  strncpy( rc_data.station_sideband, "USB",
	  sizeof(rc_data.station_sideband) );
  rc_data.station_freq = 13880600;

  return( FALSE );
} /* End of Load_Config() */

/*------------------------------------------------------------------*/

/* New_Lines_Per_Min()
 *
 * Initializes parameters on new RPM selected by user
 */
  void
New_Lines_Per_Min( void )
{
  int rpm[ NUM_RPM ] =
  { RPM60, RPM90, RPM100, RPM120, RPM180, RPM240 };

  char name[8];
  int idx;

  /* Find active RPM menu item */
  for( idx = 0; idx < NUM_RPM; idx++ )
  {
	snprintf( name, sizeof(name), "rpm%d", rpm[idx] );
	name[7] = '\0';
	if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(
			lookup_widget(gbl_popup_menu, name))) )
	  break;
  }
  if( idx == NUM_RPM ) return;

  /* Enter user selected value of RPM */
  rc_data.lines_per_min = (double)rpm[ idx ];
  Configure();

} /* New_Lines_Per_Min() */

/*------------------------------------------------------------------*/

/* New_Pixels_Per_Line()
 *
 * Initializes parameters on new line resolution
 * (pixels per line) selected by the user
 */
  void
New_Pixels_Per_Line( void )
{
  int pix[ NUM_PIX ] = { PIX600, PIX1200 };

  char name[8];
  int idx;

  /* Find active pixels per line menu item */
  for( idx = 0; idx < NUM_PIX; idx++ )
  {
	snprintf( name, sizeof(name), "pix%d", pix[idx] );
	name[7] = '\0';
	if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(
			lookup_widget(gbl_popup_menu, name))) )
	  break;
  }
  if( idx == NUM_PIX ) return;

  /* Enter user selected value of resolution */
  rc_data.pixels_per_line = pix[ idx ];
  Configure();

} /* New_Pixels_Per_Line() */

/*------------------------------------------------------------------*/

/* New_IOC()
 *
 * Initializes parameters on new
 * IOC value selected by the user
 */
  void
New_IOC( void )
{
  int ioc[ NUM_IOC ] = { IOC288, IOC576 };
  int stn[ NUM_IOC ] = { IOC288_START_TONE, IOC576_START_TONE };

  char name[8];
  int idx;

  /* Find active IOC menu item */
  for( idx = 0; idx < NUM_IOC; idx++ )
  {
	snprintf( name, sizeof(name), "ioc%d", ioc[idx] );
	name[7] = '\0';
	if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(
			lookup_widget(gbl_popup_menu, name))) )
	  break;
  }
  if( idx == NUM_IOC ) return;

  /* Enter user selected value of IOC */
  rc_data.ioc_value  = ioc[ idx ];
  rc_data.start_tone = stn[ idx ];

  /* Period of start and stop tones in pixels */
  double temp = rc_data.lines_per_min / 60.0; /* lines/sec */
  rc_data.start_tone_period =
	temp * (double)rc_data.pixels_per_line / (double)rc_data.start_tone;
  rc_data.stop_tone_period =
	temp * (double)rc_data.pixels_per_line / (double)WEFAX_STOP_TONE;

} /* New_IOC() */

/*------------------------------------------------------------------*/

/* New_Phasing_Lines()
 *
 * Initializes parameters on new number
 * of phasing lines selected by the user
 */
  void
New_Phasing_Lines( void )
{
  int phl[ NUM_PHL ] = { PHL10, PHL20, PHL40, PHL60 };

  char name[8];
  int idx;

  /* Find active phasing lines menu item */
  for( idx = 0; idx < NUM_PHL; idx++ )
  {
	snprintf( name, sizeof(name), "phl%d", phl[idx] );
	name[7] = '\0';
	if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(
			lookup_widget(gbl_popup_menu, name))) )
	  break;
  }
  if( idx == NUM_PHL ) return;

  /* Enter user selected value of phasing lines */
  rc_data.phasing_lines = phl[ idx ];

} /* New_Phasing_Lines() */

/*------------------------------------------------------------------*/

/* New_Image_Enhance()
 *
 * Initializes parameters on new image
 * enhancement algorithm selected by the user
 */
  void
New_Image_Enhance( void )
{
  int ime[ NUM_IME ] = { IME0, IME1, IME2 };

  char name[8];
  int idx;

  /* Find active phasing lines menu item */
  for( idx = 0; idx < NUM_IME; idx++ )
  {
	snprintf( name, sizeof(name), "ime%d", ime[idx] );
	name[7] = '\0';
	if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(
			lookup_widget(gbl_popup_menu, name))) )
	  break;
  }
  if( idx == NUM_IME ) return;

  /* Enter user selected value of phasing lines */
  rc_data.image_enhance = ime[ idx ];

} /* New_Image_Enhance() */

/*------------------------------------------------------------------*/

/*  Configure()
 *
 *  Initializes xwefax after change of parameters
 */
  void
Configure( void )
{
  /* The scrolled window image container */
  GtkWidget *image_scroller;

  static int pixels_per_line  = 0;
  static double lines_per_min = 0.0;
  gboolean
	new_lpm = FALSE,
	new_ppl = FALSE;


  /* Initialize on change of resolution */
  if( pixels_per_line != rc_data.pixels_per_line )
  {
	pixels_per_line = rc_data.pixels_per_line;
	rc_data.pixels_per_line2 = rc_data.pixels_per_line / 2;
	new_ppl = TRUE;
  }

  /* Initialize on change of RPM */
  if( lines_per_min  != rc_data.lines_per_min )
  {
	lines_per_min = rc_data.lines_per_min;
	new_lpm = TRUE;
  }

  /* Initialize on change of resolution */
  if( new_ppl )
  {
	/* We need a triple-sized buffer to avoid over-
	 * running pixels after the buffer's output index */
	rc_data.line_buffer_size = 2 * pixels_per_line;

	/* Allocate line buffer */
	if( !mem_realloc((void*)&gbl_line_buffer,
		  (size_t)rc_data.line_buffer_size) )
	{
	  Show_Message(
		  _("Failed to Allocate Memory to Line Buffer\n"
			"Please quit and correct"), "red" );
	  Error_Dialog(
		  _("Failed to Allocate Memory to Line Buffer\n"
			"Please Quit and correct"), QUIT );
	  return;
	}
	bzero( gbl_line_buffer, (size_t)rc_data.line_buffer_size );

	/* Create pixbuff for WEFAX images */
	if( gbl_wefax_pixbuf != NULL )
	{
	  g_object_unref( gbl_wefax_pixbuf );
	  gbl_wefax_pixbuf = NULL;
	}
	gbl_wefax_pixbuf = gdk_pixbuf_new(
		GDK_COLORSPACE_RGB, FALSE, 8,
		pixels_per_line, rc_data.image_lines );

	/* Error, not enough memory */
	if( gbl_wefax_pixbuf == NULL)
	{
	  Show_Message(
		  _("Failed to Allocate Memory to Pixbuf\n"
			"Please Quit and correct"), "red" );
	  Error_Dialog(
		  _("Failed to Allocate Memory to Pixbuf\n"
			"Please Quit and correct"), QUIT );
	  return;
	}

	/* Fill pixbuf with background color */
	gdk_pixbuf_fill( gbl_wefax_pixbuf, PIXBUF_BACKGND );

	/* Get details of pixbuf */
	gbl_pixel_buf  = gdk_pixbuf_get_pixels( gbl_wefax_pixbuf );
	gbl_rowstride  = gdk_pixbuf_get_rowstride( gbl_wefax_pixbuf );
	gbl_n_channels = gdk_pixbuf_get_n_channels( gbl_wefax_pixbuf );

	/* Globalize drawingarea to be displayed */
	gbl_wefax_drawingarea =
	  lookup_widget( main_window, "wefax_drawingarea" );
	gtk_widget_set_size_request(
		gbl_wefax_drawingarea,
		pixels_per_line,
		rc_data.image_lines );
	gtk_widget_show( gbl_wefax_drawingarea );

	/* Set window size as required */
	image_scroller = lookup_widget( main_window, "image_scrolledwindow" );
	gtk_widget_set_size_request(
		image_scroller, -1,
		rc_data.window_height );
	gtk_window_resize( GTK_WINDOW(main_window), 10, 10 );

	/* Re-initialize line buffer indices */
	gbl_linebuff_input  = 0;
	gbl_linebuff_output =
	  rc_data.line_buffer_size - rc_data.pixels_per_line2;

	/* Signal WEFAX decoder to reset */
	SetFlag( START_NEW_IMAGE );
  } /* if( new_ppl ) */

  /* Initialize on change of RPM or resolution */
  if( new_lpm || new_ppl )
  {
	new_lpm = FALSE;
	new_ppl = FALSE;

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

	/* Period of start and stop tones in pixels */
	temp = lines_per_min / 60.0; /* lines/sec */
	rc_data.start_tone_period =
	  temp * (double)pixels_per_line / (double)rc_data.start_tone;
	rc_data.stop_tone_period =
	  temp * (double)pixels_per_line / (double)WEFAX_STOP_TONE;
  } /* if( if( new_lpm || new_ppl ) */

} /* Configure() */

/*------------------------------------------------------------------------*/

/*  File_Name_jpg()
 *
 *  Prepare a file name, using date and time
 */
  void
File_Name( char *file_name, const char *extn )
{
  int len; /* String length of file_name */

  /* Variables for reading time (UTC) */
  time_t tp;
  struct tm utc;

  /* Prepare a file name as UTC date-time. */
  /* Default paths are images/ and record/ */
  time( &tp );
  utc = *gmtime( &tp );
  Strlcpy( file_name, rc_data.xwefax_dir, MAX_FILE_NAME - 28 );
  len = (int)strlen( file_name );
  strftime( &file_name[len], 25, "images/%d%b%Y-%H%Mz.", &utc );
  len = (int)strlen( file_name );
  Strlcat( &file_name[len], extn, 4 );

} /* End of File_Name() */

/*------------------------------------------------------------------------*/

/* Fname()
 *
 * Finds file name in a file path
 */
  char *
Fname( char *fpath )
{
  int idx;

  idx = (int)strlen( fpath );

  while( (--idx >= 0) && (fpath[idx] != '/') );

  return( &fpath[++idx] );

} /* Fname() */

/*------------------------------------------------------------------------*/

/* Open_File()
 *
 * Opens a file, aborts on error
 */
  gboolean
Open_File( FILE **fp, char *fname, const char *mode )
{
  /* Message buffer */
  char mesg[64];

  /* Open Channel A image file */
  *fp = fopen( fname, mode );
  if( *fp == NULL )
  {
	perror( fname );
	snprintf( mesg, sizeof(mesg),
		_("Failed to open file: %s"), Fname(fname) );
	Show_Message( mesg, "red" );
	Error_Dialog( mesg, QUIT );
	return( FALSE );
  }

  return( TRUE );
} /* End of Open_File() */

/*------------------------------------------------------------------------*/

/* Save_Image_pgm()
 *
 * Write an image buffer to file
 */
  gboolean
Save_Image_pgm(
	FILE *fp, const char *type,
	int width, int height,
	int max_val, unsigned char *buffer )
{
  size_t size;

  /* Write header in Ch-A output PGM files */
  if( fprintf(fp, "%s\n%s\n%d %d\n%d\n",
		type, _("# Created by xwefax"), width, height, max_val) < 0 )
  {
	fclose( fp );
	perror( "xwefax: Error writing Image to file" );
	Set_Indicators( ICON_SAVE_NO );
	Show_Message( _("Error writing Image to file"), "red" );
	Error_Dialog( _("Error writing Image to file"), QUIT );
	return( FALSE );
  }

  /* P6 type (PGM) files are 3* size in pixels */
  if( strcmp(type, "P6") == 0 )
	size = (size_t)(3 * width * height);
  else
	size = (size_t)(width * height);

  /* Write image buffer to file, abort on error */
  if( fwrite(buffer, 1, size, fp) != size )
  {
	fclose( fp );
	perror( "xwefax: Error writing Image to file" );
	Set_Indicators( ICON_SAVE_NO );
	Show_Message( _("Error writing Image to file"), "red" );
	Error_Dialog( _("Error writing Image to file"), QUIT );
	return( FALSE );
  }

  Set_Indicators( ICON_SAVE_APPLY );
  Show_Message( _("Image File saved OK"), "green" );
  fclose( fp );
  return( TRUE );
} /* Save_Image_pgm() */

/*------------------------------------------------------------------------*/

/* Save_Image_jpg()
 *
 * Write an image buffer to file
 */
  gboolean
Save_Image_jpg( FILE *fp, int width, int height, uint8_t *buffer )
{
  int len, temp;
  size_t siz;
  uint8_t *buff = NULL;

  /* Make the buffer size is multiple of 8x8 block */
  temp = 8 - (height & 0x7);

  /* Allocate temp buffer to multiple of 8x8 */
  siz = (size_t)( width * (height + temp) ) * sizeof(uint8_t);
  if( !mem_alloc((void **)&buff, siz) )
  {
	Set_Indicators( ICON_SAVE_NO );
	return( FALSE );
  }

  /* Clear temp buffer and copy image data */
  memset( buff, 0xff, siz );
  siz = (size_t)( width * height ) * sizeof(uint8_t);
  memcpy( buff, buffer, siz );

  /* Create a jpeg encoder */
  jpec_enc_t *enc =
	jpec_enc_new( buff, (uint16_t)width, (uint16_t)(height + temp) );

  /* Run encoder to create jpeg image */
  const uint8_t *jpeg = jpec_enc_run( enc, &len );
  siz = (size_t)len;

  /* Write image buffer to file, abort on error */
  if( fwrite(jpeg, sizeof(uint8_t), siz, fp) != siz )
  {
	fclose( fp );
	perror( "xwefax: Error writing Image to file" );
	Set_Indicators( ICON_SAVE_NO );
	Show_Message( _("Error writing Image to file"), "red" );
	Error_Dialog( _("Error writing Image to file"), QUIT );
	return( FALSE );
  }

  Set_Indicators( ICON_SAVE_APPLY );
  Show_Message( _("Image File saved OK"), "green" );
  jpec_enc_del( enc );
  free_ptr( (void **)&buff );
  fclose( fp );

  return( TRUE );
} /* Save_Image_jpg() */

/*------------------------------------------------------------------------*/

/*  Usage()
 *
 *  Prints usage information
 */
  void
Usage( void )
{
  fprintf( stderr, "%s\n",
	  _("Usage: xwefax [-hv]") );

  fprintf( stderr, "%s\n",
	  _("       -h: Print this usage information and exit"));

  fprintf( stderr, "%s\n",
	  _("       -v: Print version number and exit"));

} /* End of Usage() */

/*------------------------------------------------------------------------*/

/*  Show_Message()
 *
 *  Prints a message string in the Text View scroller
 */
  void
Show_Message( char *mesg, char *attr )
{
  GtkAdjustment *adjustment;

  static GtkTextIter iter;
  static gboolean first_call = TRUE;

  /* Initialize */
  if( first_call )
  {
	first_call = FALSE;
	gtk_text_buffer_get_iter_at_offset( gbl_text_buffer, &iter, 0 );
  }

  /* Print message */
  gtk_text_buffer_insert_with_tags_by_name(
	  gbl_text_buffer, &iter,
	  mesg, -1, attr, NULL );
  gtk_text_buffer_insert( gbl_text_buffer, &iter, "\n", -1 );

  /* Scroll Text View to bottom */
  adjustment = gtk_scrolled_window_get_vadjustment
	( GTK_SCROLLED_WINDOW(gbl_text_scroller) );
  gtk_adjustment_set_value(
	  adjustment, adjustment->upper - adjustment->page_size );

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );

} /* End of Show_Message() */

/*------------------------------------------------------------------------*/

/***  Memory allocation/freeing utils ***/
gboolean mem_alloc( void **ptr, size_t req )
{
  free_ptr( ptr );
  *ptr = malloc( req );

  if( *ptr == NULL )
  {
	perror( "xwefax: alloc():" );
	Error_Dialog( _("A memory allocation failed\n"\
		  "Please quit xwefax and correct"), QUIT  );
	return( FALSE );
  }

  return( TRUE );
} /* End of void mem_alloc() */

/*------------------------------------------------------------------------*/

gboolean mem_realloc( void **ptr, size_t req )
{
  *ptr = realloc( *ptr, req );
  if( *ptr == NULL )
  {
	perror( "xwefax: realloc():" );
	Error_Dialog( _("A memory re-allocation failed\n"\
		  "Please quit xwefax and correct"), QUIT  );
	return( FALSE );
  }
  return( TRUE );
} /* End of void mem_realloc() */

/*------------------------------------------------------------------------*/

void free_ptr( void **ptr )
{
  if( *ptr != NULL ) free( *ptr );
  *ptr = NULL;

} /* End of void free_ptr() */

/*------------------------------------------------------------------------*/

/*  Cleanup()
 *
 *  Cleanup before quitting or not using sound card
 */
  void
Cleanup( void )
{
  if( rc_data.tcvr_type == PERSEUS )
	Perseus_Close_Device();
  else
  {
	Close_Tcvr_Serial();
	Close_Capture();
  }

  if( gbl_dsp_buffer != NULL )
  {
	free( gbl_dsp_buffer );
	gbl_dsp_buffer = NULL;
  }

  gbl_image_file[0] = '\0';

} /*  Cleanup() */

/*------------------------------------------------------------------------*/

/* Functions for testing and setting/clearing flags */

/* An int variable holding the single-bit flags */
static int Flags = 0;

  int
isFlagSet(int flag)
{
  return (Flags & flag);
}

  int
isFlagClear(int flag)
{
  return (~Flags & flag);
}

  void
SetFlag(int flag)
{
  Flags |= flag;
}

  void
ClearFlag(int flag)
{
  Flags &= ~flag;
}

  void
ToggleFlag(int flag)
{
  Flags ^= flag;
}

/*------------------------------------------------------------------*/

/* Strlcpy()
 *
 * Copies n-1 chars from src string into dest string. Unlike other
 * such library fuctions, this makes sure that the dest string is
 * null terminated by copying only n-1 chars to leave room for the
 * terminating char. n would normally be the sizeof(dest) string but
 * copying will not go beyond the terminating null of src string
 */
  void
Strlcpy( char *dest, const char *src, size_t n )
{
  char ch = src[0];
  int idx = 0;

  /* Leave room for terminating null in dest */
  n--;

  /* Copy till terminating null of src or to n-1 */
  while( (ch != '\0') && (n > 0) )
  {
	dest[idx] = src[idx];
	idx++;
	ch = src[idx];
	n--;
  }

  /* Terminate dest string */
  dest[idx] = '\0';

} /* Strlcpy() */

/*------------------------------------------------------------------*/

/* Strlcat()
 *
 * Concatenates at most n-1 chars from src string into dest string.
 * Unlike other such library fuctions, this makes sure that the dest
 * string is null terminated by copying only n-1 chars to leave room
 * for the terminating char. n would normally be the sizeof(dest)
 * string but copying will not go beyond the terminating null of src
 */
  void
Strlcat( char *dest, const char *src, size_t n )
{
  char ch = dest[0];
  int idd = 0; /* dest index */
  int ids = 0; /* src  index */

  /* Find terminating null of dest */
  while( (n > 0) && (ch != '\0') )
  {
	idd++;
	ch = dest[idd];
	n--; /* Count remaining char's in dest */
  }

  /* Copy n-1 chars to leave room for terminating null */
  n--;
  ch = src[ids];
  while( (n > 0) && (ch != '\0') )
  {
	dest[idd] = src[ids];
	ids++;
	ch = src[ids];
	idd++;
	n--;
  }

  /* Terminate dest string */
  dest[idd] = '\0';

} /* Strlcat() */

/*------------------------------------------------------------------*/

