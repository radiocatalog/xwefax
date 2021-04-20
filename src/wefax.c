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

#include "wefax.h"
#include "shared.h"

/*------------------------------------------------------------------------*/

/* Receive_Error()
 *
 * Displays error conditions on receive
 */
  static void
Receive_Error( void )
{
  Show_Message( _("Error - Stopping Reception"), "red" );
  Set_Indicators( ICON_START_NO );
  Set_Indicators( ICON_SYNC_NO );
  Set_Indicators( ICON_DECODE_NO );
  Close_Capture();
} /* Receive_Error() */

/*------------------------------------------------------------------------*/

/* Wefax_Decode()
 *
 * Function that decodes Wefax signals into images
 */
  static gboolean
Wefax_Decode( void )
{
  /* First call of function flag */
  static gboolean first_call = TRUE;

  /* Pixbuf's pixel pointer */
  guchar *pixel;

  /* Buffer for creating a PGM image file */
  static unsigned char *image_buffer = NULL;
  int image_buffer_idx;  /* Index to above */
  size_t buf_size;

  unsigned char discr_op;     /* Detector output */
  static double discr_op_ave; /* Detector output average */
  int
    discr_op_max  = 0,    /* Detector output maximum */
    discr_max_idx = 0;    /* Detector output max's index */

  int
    norm_idx,   /* Index to image buffer for normalization */
    norm_len;   /* Length of image line to be normalized */

  static int
    pixel_idx,    /* Index to current pixel in image line */
    sync_pos_ref, /* Position ref of sync pulse, tracks input idx */
    sync_correct; /* Count of up or down sync error directions */

  /* Distance in pix of sync pulse from its required position */
  int sync_error;

  /* File name string for saving images */
  static char
    file_name_jpg[ MAX_FILE_NAME ],
    file_name_pgm[ MAX_FILE_NAME ];

  /* File pointer for above */
  FILE *fp = NULL;

  /* Stop Tone received flag */
  static gboolean stop = FALSE;

  /* Reset on new params */
  if( isFlagSet(START_NEW_IMAGE) )
  {
    first_call = TRUE;
    ClearFlag( START_NEW_IMAGE );
  }

  /*** Initialize ***/
  if( first_call )
  {
    Set_Indicators( ICON_DECODE_YES );
    if( isFlagSet(SAVE_IMAGE) )
      Set_Indicators( ICON_SAVE_YES );

    /* Make a file name for the WEFAX image */
    File_Name( file_name_jpg, "jpg" );
    File_Name( file_name_pgm, "pgm" );

    /* Fill pixbuf with background color */
    gdk_pixbuf_fill( wefax_pixbuf, PIXBUF_BACKGND );
    gtk_widget_queue_draw( wefax_drawingarea );

    /* Initialize statics */
    pixel_idx = 0;
    line_count = 0;
    discr_op_ave = 0.0;
    first_call = FALSE;
    stop = FALSE;

    /* Allocate image buffer */
    free_ptr( (void **)&image_buffer );
    buf_size = (size_t)rc_data.pixels_per_line;
    if( !mem_alloc((void **)&image_buffer, buf_size) )
    {
      Show_Message(
          _("Memory Allocation failed\n"
            "for image buffer"), "red" );
      return( FALSE );
    }

  } /* if( first_call ) */

  /* Stop on user request */
  if( isFlagSet(RECEIVE_STOP) && line_count )
  {
    /* Open file and save WEFAX PGM image */
    if( isFlagSet(SAVE_IMAGE_PGM) && isFlagSet(SAVE_IMAGE) )
    {
      Show_Message( _("Saving Decoded PPM Image File ..."), "black" );
      if( !Open_File(&fp, file_name_pgm, "w") ||
          !Save_Image_PGM(fp, "P5", rc_data.pixels_per_line,
            line_count, 255, image_buffer) )
      {
        Show_Message( _("Failed to save PPM Image File"), "red" );
        Set_Indicators( ICON_DECODE_NO );
      }
    } /* if( isFlagSet(SAVE_IMAGE_PGM) ) */

    /* Open file and save WEFAX JPEG image */
    if( isFlagSet(SAVE_IMAGE_JPG) && isFlagSet(SAVE_IMAGE) )
    {
      Show_Message( _("Saving Decoded JPG Image File ..."), "black" );
      if( !Open_File(&fp, file_name_jpg, "w") ||
          !Save_Image_JPEG(fp, rc_data.pixels_per_line,
            line_count, image_buffer) )
      {
        Show_Message( _("Failed to save JPG Image File"), "red" );
        Set_Indicators( ICON_DECODE_NO );
      }
    } /* if( isFlagSet(SAVE_IMAGE_JPG) ) */

    first_call   = TRUE;
    wefax_action = ACTION_STOP;
    return( TRUE );
  } /* if( isFlagSet(RECEIVE_STOP) ) */

  /* Skip looking for start tones */
  if( isFlagSet(SKIP_ACTION) )
  {
    /* Re-initialize line buffer indices */
    linebuff_input  = 0;
    linebuff_output =
      rc_data.line_buffer_size - rc_data.pixels_per_line2;

    Show_Message( _("Skipping Image Decode"), "orange" );
    Set_Indicators( ICON_DECODE_SKIP );
    ClearFlag( SKIP_ACTION );

    if( line_count )
    {
      /* Open file and save WEFAX PGM image */
      if( isFlagSet(SAVE_IMAGE_PGM) && isFlagSet(SAVE_IMAGE) )
      {
        Show_Message( _("Saving Decoded PPM Image File ..."), "black" );
        if( !Open_File(&fp, file_name_pgm, "w") ||
            !Save_Image_PGM(fp, "P5", rc_data.pixels_per_line,
              line_count, 255, image_buffer) )
        {
          Show_Message( _("Failed to save PPM Image File"), "red" );
          Set_Indicators( ICON_DECODE_NO );
        }
      } /* if( isFlagSet(SAVE_IMAGE_PGM) ) */

      /* Open file and save WEFAX JPEG image */
      if( isFlagSet(SAVE_IMAGE_JPG) && isFlagSet(SAVE_IMAGE) )
      {
        Show_Message( _("Saving Decoded JPG Image File ..."), "black" );
        if( !Open_File(&fp, file_name_jpg, "w") ||
            !Save_Image_JPEG(fp, rc_data.pixels_per_line,
              line_count, image_buffer) )
        {
          Show_Message( _("Failed to save JPG Image File"), "red" );
          Set_Indicators( ICON_DECODE_NO );
        }
      } /* if( isFlagSet(SAVE_IMAGE_JPG) ) */

    } /* if( line_count ) */

    first_call   = TRUE;
    wefax_action = ACTION_BEGIN;
    return( TRUE );
  } /* if( isFlagSet(SKIP_ACTION) ) */

  /* Fill the image line buffer */
  /* Take a sample from the FM detector */
  if( !FM_Detector(&discr_op) )
    return( FALSE );

  /* Display detector output */
  if( isFlagSet(DISPLAY_SIGNAL) )
    Display_Signal( discr_op );

  /* Detect stop pulse */
  stop |= Stop_Tone_Detect( discr_op );

  /* Current position in image buffer to save pixel value */
  line_buffer[ linebuff_input ] = discr_op;
  linebuff_input++;
  if( linebuff_input >= rc_data.line_buffer_size )
    linebuff_input = 0;

  /* Return to control function at each pixel */
  pixel_idx++;
  if( pixel_idx < rc_data.pixels_per_line )
    return( TRUE );

  /* Copy line buffer to currrent image buffer line */
  discr_max_idx = 0;
  discr_op_max  = -256;
  image_buffer_idx = line_count * rc_data.pixels_per_line;
  for( pixel_idx = 0; pixel_idx < rc_data.pixels_per_line; pixel_idx++ )
  {
    /* Try to sync image if enabled by finding
     * the position of sync pulse maximum */
    if( isFlagSet(INIMAGE_PHASING) &&
        (pixel_idx < INIMAGE_PHASING_RANGE) )
    {
      /* Average negated line buffer pixel values */
      discr_op_ave *= PHASING_PUSLE_WIN - 1.0;
      discr_op_ave -= (double)line_buffer[ linebuff_output ];
      discr_op_ave /= PHASING_PUSLE_WIN;

      /* Record maximum value of detector output */
      if( discr_op_max < (int)discr_op_ave )
      {
        discr_op_max  = (int)discr_op_ave;
        discr_max_idx = pixel_idx;
      }
    } /* if( isFlagSet(INIMAGE_PHASING) ) */

    /* Make image bi-level if enabled */
    if( rc_data.image_enhance == ENHANCE_BILEVEL )
    {
      if( line_buffer[ linebuff_output ] > BILEVEL_THRESHOLD )
        line_buffer[ linebuff_output ] = 255;
      else
        line_buffer[ linebuff_output ] = 0;
    }

    /* Copy line buffer to currrent image buffer line */
    image_buffer[ image_buffer_idx + pixel_idx ] =
      line_buffer[ linebuff_output ];
    linebuff_output++;
    if( linebuff_output >= rc_data.line_buffer_size )
      linebuff_output = 0;
  } /* for( pixel_idx = 0; pixel_idx < rc_data.pixels_per ... */

  if( isFlagClear(INIMAGE_PHASING) )
    sync_correct = 0;

  /* Correct sync error one pixel at a time if enabled */
  if( isFlagSet(INIMAGE_PHASING) &&
      (discr_op_max > INIMAGE_SYNC_THRESHOLD) )
  {
    /* Try to set the sync pulse at start of line */
    sync_error = discr_max_idx - PHASING_PULSE_REF;

    /* Count up or down error conditions */
    if( sync_error > 0 ) sync_correct--;
    if( sync_error < 0 ) sync_correct++;

    /* Keep sync correction inside the
     * sync correct range to avoid hunting */
    if( sync_correct >= SYNC_CORRECT_RANGE )
    {
      linebuff_input++;
      sync_pos_ref++;
      sync_correct = 0;
    }
    else if( sync_correct <= -SYNC_CORRECT_RANGE )
    {
      linebuff_input--;
      sync_pos_ref--;
      sync_correct = 0;
    }

    /* Keep buffer index within bounds */
    if( linebuff_input >= rc_data.line_buffer_size )
      linebuff_input -= rc_data.line_buffer_size;
    else if( linebuff_input < 0 )
      linebuff_input += rc_data.line_buffer_size;

  } /* if( isFlagSet(INIMAGE_PHASING) ) */

  /* Normalize image line for better contrast.
   * Leave behind the pixels of phasing pulse. */
  if( rc_data.image_enhance == ENHANCE_CONTRAST )
  {
    norm_idx = image_buffer_idx + PHASING_PULSE_LEN;
    norm_len = rc_data.pixels_per_line - PHASING_PULSE_LEN;
    Normalize( &image_buffer[norm_idx], norm_len );
  }

  /* Fill pixels of display buffer from the image line */
  for( pixel_idx = 0; pixel_idx < rc_data.pixels_per_line; pixel_idx++ )
  {
    pixel =
      pixel_buf +
      n_channels * pixel_idx +
      line_count * rowstride;

    pixel[0] = image_buffer[ image_buffer_idx + pixel_idx ];
    pixel[1] = image_buffer[ image_buffer_idx + pixel_idx ];
    pixel[2] = image_buffer[ image_buffer_idx + pixel_idx ];
  } /* for( pixel_idx = 0; pixel_idx < rc_data.pixels_per ... */

  /* Draw the (partial) image */
  gtk_widget_queue_draw( wefax_drawingarea );

  /* Wait for GTK to complete its tasks */
  while( g_main_context_iteration(NULL, FALSE) );

  /* Make sure that the buffer input
   * index stays ahead of output index */
  int diff = linebuff_input - linebuff_output;
  if( (diff < 0) && (diff >= -rc_data.pixels_per_line) )
    linebuff_input += rc_data.pixels_per_line;
  else if( diff > rc_data.pixels_per_line )
    linebuff_input -= rc_data.pixels_per_line;

  if( linebuff_input >= rc_data.line_buffer_size )
    linebuff_input -= rc_data.line_buffer_size;
  else if( linebuff_input < 0 )
    linebuff_input += rc_data.line_buffer_size;

  /* End image decode on stop tone */
  if( stop )
  {
    Show_Message( _("Ending WEFAX Decode ..."), "green" );
    Set_Indicators( ICON_DECODE_APPLY );
  }

  /* End image decode if gone for
   * too long (missed stop tone?) */
  line_count++;
  if( line_count >= rc_data.image_lines )
  {
    Show_Message( _("Ending Decode-Missed Stop Tone?"), "orange" );
    Set_Indicators( ICON_DECODE_SKIP );
    stop = TRUE;
  }

  /* End image decode and save */
  if( stop && line_count )
  {
    /* Open file and save WEFAX JPEG image */
    if( isFlagSet(SAVE_IMAGE_JPG) && isFlagSet(SAVE_IMAGE) )
    {
      Show_Message( _("Saving Decoded JPG Image File ..."), "black" );
      if( !Open_File(&fp, file_name_jpg, "w") ||
          !Save_Image_JPEG(fp, rc_data.pixels_per_line,
            line_count, image_buffer) )
      {
        Show_Message( _("Failed to save JPG Image File"), "red" );
        Set_Indicators( ICON_DECODE_NO );
      }
    } /* if( isFlagSet(SAVE_IMAGE_JPG) ) */

    /* Open file and save WEFAX PGM image */
    if( isFlagSet(SAVE_IMAGE_PGM) && isFlagSet(SAVE_IMAGE) )
    {
      Show_Message( _("Saving Decoded PPM Image File ..."), "black" );
      if( !Open_File(&fp, file_name_pgm, "w") ||
          !Save_Image_PGM(fp, "P5", rc_data.pixels_per_line,
            line_count, 255, image_buffer) )
      {
        Show_Message( _("Failed to save PPM Image File"), "red" );
        Set_Indicators( ICON_DECODE_NO );
      }
    } /* if( isFlagSet(SAVE_IMAGE_PGM) ) */

    first_call   = TRUE;
    wefax_action = ACTION_BEGIN;
    return( TRUE );
  } /* if( stop && line_count && isFlagSet(SAVE_IMAGE) ) */

  /* Re-allocate image buffer per line */
  buf_size = (size_t)( (line_count + 1) * rc_data.pixels_per_line );
  if( !mem_realloc((void **)&image_buffer, buf_size) )
  {
    Show_Message(
        _("Memory Allocation failed\n"
          "for image buffer"), "red" );
    return( FALSE );
  }

  pixel_idx = 0;
  return( TRUE );
} /* Wefax_Decode() */

/*------------------------------------------------------------------------*/

/* Wefax_Control()
 *
 * Central control function that
 * directs Wefax decoding functions
 */
  static gboolean
Wefax_Control( gpointer data )
{
  int error; /* Returns error numbers */
  char mesg[MESG_SIZE]; /* Messages string for display */

  /* Adjustments for setting the position of scroller */
  GtkAdjustment *adjm;
  GtkScrolledWindow *scrollwin;


  /* Initialize Perseus SDR if selected */
  if( rc_data.tcvr_type == PERSEUS )
  {
#ifdef HAVE_LIBPERSEUS_SDR
    if( isFlagClear(PERSEUS_INIT) )
    {
      if( !Perseus_Initialize() )
        return( FALSE );
    }
#endif
  }
  else
  {
    /* Setup sound card if needed, abort on error */
    if( isFlagClear(CAPTURE_SETUP) )
    {
      mesg[0] = '\0';
      if( !Open_Capture( mesg, &error ) )
      {
        if( error )
        {
          Strlcat( mesg, _("\nError: "), sizeof(mesg) );
          Strlcat( mesg, snd_strerror(error), sizeof(mesg) );
        }

        Close_Capture();
        Error_Dialog( mesg, QUIT );
        return( FALSE );
      } /* if( !Open_Capture() ) */
    } /* if( isFlagClear(CAPTURE_SETUP) ) */

    /* Setup CAT if enabled */
    if( isFlagSet(ENABLE_CAT) && isFlagClear(CAT_SETUP) )
      Open_Tcvr_Serial();
  } /* else of if( rc_data.tcvr_type == PERSEUS ) */

  /* Save stations file if requested by user */
  if( isFlagSet(SAVE_STATIONS) )
    Save_Stations_File( rc_data.stations_file );

  /* Direct program flow according
   * to currently selected action */
  switch( wefax_action )
  {
    case ACTION_BEGIN: /* Begin decoding process */
      Show_Message( _("Listening for Start Tone ..."), "black" );
      Set_Indicators( ICON_START_YES );
      Set_Indicators( ICON_SYNC_NO );

      /* Move scroller to top of image window */
      scrollwin = GTK_SCROLLED_WINDOW(
          Builder_Get_Object(main_window_builder, "image_scrolledwindow") );
      adjm = gtk_scrolled_window_get_vadjustment( scrollwin );
      gtk_adjustment_set_value( adjm, 0.0 );

      wefax_action = ACTION_START;
      break;

    case ACTION_START: /* Looking for WEFAX start tone */
      if( !Start_Tone_Detect() )
      {
        Receive_Error();
        return( FALSE );
      }
      break;

    case ACTION_PHASING: /* Sync with WEFAX phasing pulses */
      if( !Phasing_Detect() )
      {
        Receive_Error();
        return( FALSE );
      }
      break;

    case ACTION_DECODE: /* Decode WEFAX images */
      if( !Wefax_Decode() )
      {
        Receive_Error();
        return( FALSE );
      }
      break;

    case ACTION_STOP: /* Stop operations */
      Show_Message( _("Stopping Reception"), "black" );
      if( isFlagSet(CAPTURE_SETUP) )
        Close_Capture();
      if( isFlagSet(CAT_SETUP) )
        Close_Tcvr_Serial();

#ifdef HAVE_LIBPERSEUS_SDR
      if( isFlagSet(PERSEUS_INIT) )
        Perseus_Close_Device();
#endif
      return( FALSE );

  } /* switch( wefax_action ) */

  return( TRUE );
} /* Wefax_Control() */

/*------------------------------------------------------------------------*/

/* Wefax_Drawingarea_Button_Press()
 *
 * Handles button press event on wefax drawingarea
 */
  gboolean
Wefax_Drawingarea_Button_Press( GdkEventButton  *event )
{
  /* Popup main menu */
  if( event->button == 3 )
  {
    gtk_menu_popup_at_pointer( GTK_MENU(popup_menu), NULL );
    return TRUE;
  }

  /* Adjust the line buffer input index to bring the
   * column of button press to the beginnig of line */
  if( event->button == 1 )
  {
    linebuff_input -= (int)(event->x + 0.5);
    if( linebuff_input < 0 )
      linebuff_input += rc_data.line_buffer_size;
 }

  return TRUE;
} /* Wefax_Drawingarea_Button_Press() */

/*------------------------------------------------------------------------*/

/* Start_Button_Toggled()
 *
 * Handles toggled event of START toggle button
 * Sets menu item sensitivity and various indicators
 */
  void
Start_Button_Toggled( GtkToggleButton *togglebutton )
{
  GtkLabel *lbl = GTK_LABEL(
      Builder_Get_Object(main_window_builder, "rcve_status") );
  if( gtk_toggle_button_get_active(togglebutton) )
  {
    ClearFlag( RECEIVE_STOP );
    gtk_label_set_markup( lbl, RECEIVE );
    Set_Indicators( ICON_SYNC_NO );
    Set_Indicators( ICON_DECODE_NO );
    linebuff_input  = 0;
    linebuff_output =
      rc_data.line_buffer_size - rc_data.pixels_per_line2;
    wefax_action = ACTION_BEGIN;
    g_idle_add( Wefax_Control, NULL );
  }
  else
  {
    SetFlag( RECEIVE_STOP );
    Set_Indicators( ICON_START_NO );
    Set_Indicators( ICON_SYNC_NO );
    Set_Indicators( ICON_DECODE_NO );
    gtk_label_set_markup( lbl, STANDBY );
  }
} /* Start_Button_Toggled() */

/*------------------------------------------------------------------------*/

