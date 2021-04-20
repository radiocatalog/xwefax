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

#include "main.h"
#include "shared.h"

/* Signal handler */
static void sig_handler( int signal );

/*------------------------------------------------------------------------*/

int
main (int argc, char *argv[])
{
  /* Command line option returned by getopt() */
  int option;

  /* New and old actions for sigaction() */
  struct sigaction sa_new, sa_old;


  /* Initialize new actions */
  sa_new.sa_handler = sig_handler;
  sigemptyset( &sa_new.sa_mask );
  sa_new.sa_flags = 0;

  /* Register function to handle signals */
  sigaction( SIGINT,  &sa_new, &sa_old );
  sigaction( SIGSEGV, &sa_new, 0 );
  sigaction( SIGFPE,  &sa_new, 0 );
  sigaction( SIGTERM, &sa_new, 0 );
  sigaction( SIGABRT, &sa_new, 0 );

  /* Process command line options */
  while( (option = getopt(argc, argv, "hv") ) != -1 )
    switch( option )
    {
      case 'h' : /* Print usage and exit */
        Usage();
        return(0);

      case 'v' : /* Print version */
        puts( PACKAGE_STRING );
        return(0);

      default: /* Print usage and exit */
        Usage();
        exit(-1);

    } /* End of switch( option ) */

#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  gtk_init (&argc, &argv);

  /* Create file path to xwefax glade file */
  Strlcpy( rc_data.xwefax_glade,
      getenv("HOME"), sizeof(rc_data.xwefax_glade) );
  Strlcat( rc_data.xwefax_glade,
      "/xwefax/xwefax.glade", sizeof(rc_data.xwefax_glade) );

  /* Check for the Glade config file */
  FILE *fp = fopen( rc_data.xwefax_glade, "r" );
  if( fp == NULL )
  {
    fprintf( stderr, "xwefax: cannot open xwefax Glade GUI Description file.\n" );
    perror( rc_data.xwefax_glade );
    fprintf( stderr, "xwefax: trying to create xwefax config directory "
                     "from the installation prefix file tree.\n" );

    /* Find the binary's path (location in file system) */
    char exe_path[256], file_path[288];
    
    /* Read the file path to xwefax executable */
    size_t len = sizeof( exe_path );
    int bytes = (int)readlink( "/proc/self/exe", exe_path, len );
    if( bytes <= 0 )
    {
      fprintf( stderr, "xwefax: cannot read xwefax binary's location.\n" );
      perror( "/proc/self/exe" );
      exit( -1 );
    }

    /* Remove "/bin/xwefax" from the path with room for termination */
    bytes -= sizeof( "/bin/xwefax" ) - 1;
    if( bytes < 1 )
    {
      fprintf( stderr, "xwefax: cannot create file path to examples/xwefax.\n" );
      exit( -1 );
    }

    /* Create file path to xwefax examples directory */
    exe_path[bytes] = '\0';
    Strlcpy( file_path, exe_path, sizeof(file_path) );
    Strlcat( file_path, "/share/examples/xwefax", sizeof(file_path) );
    fprintf( stderr, "xwefax: creating xwefax config directory from: %s\n", file_path );

    /* Create system command to copy examples/xwefax to ~/.xwefax */
    char syscmnd[512];
    Strlcpy( syscmnd, "cp -r ", sizeof(syscmnd) );
    Strlcat( syscmnd, file_path, sizeof(syscmnd) );
    Strlcat( syscmnd, " ", sizeof(syscmnd) );
    Strlcat( syscmnd, getenv("HOME"), sizeof(syscmnd) );
    Strlcat( syscmnd, "/xwefax",   sizeof(syscmnd) );
    int ret = system( syscmnd );
    if( ret == -1 )
    {
      fprintf( stderr,"xwefax: cannot create xwefax's working directory.\n" );
      perror( file_path );
      exit( -1 );
    }
  } /* if( fp == NULL ) */
  else fclose( fp );

  /* Create main window and menu */
  main_window = create_main_window( &main_window_builder );
  gtk_window_set_title( GTK_WINDOW(main_window), PACKAGE_STRING );
  gtk_widget_show( main_window );

  /* Get scope widgets */
  scope_drawingarea =
    Builder_Get_Object( main_window_builder, "scope_drawingarea" );
  spectrum_drawingarea =
    Builder_Get_Object( main_window_builder, "spectrum_drawingarea" );

  /* Get sizes of displays */
  GtkAllocation alloc;
  gtk_widget_get_allocation( scope_drawingarea, &alloc );
  scope_width  = alloc.width;
  scope_height = alloc.height;
  gtk_widget_get_allocation( spectrum_drawingarea, &alloc );
  Spectrum_Size_Allocate( alloc.width, alloc.height );

  /* Create the text view scroller */
  text_scroller =
    Builder_Get_Object( main_window_builder, "text_scrolledwindow" );

  /* Get text buffer */
  text_buffer = gtk_text_view_get_buffer
    ( GTK_TEXT_VIEW(Builder_Get_Object(main_window_builder, "textview")) );

  /* Create the popup menu to set it up */
  popup_menu = create_popup_menu( &popup_menu_builder );

  /* Create some rendering tags */
  gtk_text_buffer_create_tag( text_buffer, "black",
      "foreground", "black", NULL);
  gtk_text_buffer_create_tag( text_buffer, "red",
      "foreground", "red", NULL);
  gtk_text_buffer_create_tag( text_buffer, "orange",
      "foreground", "orange", NULL);
  gtk_text_buffer_create_tag( text_buffer, "green",
      "foreground", "darkgreen", NULL);
  gtk_text_buffer_create_tag( text_buffer, "bold",
      "weight", PANGO_WEIGHT_BOLD, NULL);

  /* Make a label for the start button */
  GtkLabel *label = GTK_LABEL(
      Builder_Get_Object(main_window_builder, "rcve_status") );
  gtk_label_set_width_chars( label, 9 );
  gtk_label_set_text ( label, _(" STANDBY ") );

  /* Clear image file name */
  image_file[0] = '\0';
  rc_data.sync_slant = 0.0;

  /* Print greeting message */
  char ver[24];
  snprintf( ver, sizeof(ver), _("Welcome to %s"), PACKAGE_STRING );
  Show_Message( ver, "bold" );
  SetFlag( RECEIVE_STOP );
  SetFlag( DISPLAY_SIGNAL );
  SetFlag( SAVE_IMAGE_JPG );
  SetFlag( SAVE_IMAGE );
  Set_Indicators( ICON_SAVE_YES );

  /* Load runtime config file, abort on error */
  g_idle_add( Load_Config, NULL );

  gtk_main ();

  return 0;
} /* main() */

/*------------------------------------------------------------------------*/

/*  sig_handler()
 *
 *  Signal Action Handler function
 */

static void sig_handler( int signal )
{
  Cleanup();

  fprintf( stderr, "\n" );
  switch( signal )
  {
    case SIGINT :
      fprintf( stderr, "%s\n", _("xwefax: Exiting via User Interrupt") );
      exit(-1);

    case SIGSEGV :
      fprintf( stderr, "%s\n", _("xwefax: Segmentation Fault") );
      exit(-1);

    case SIGFPE :
      fprintf( stderr, "%s\n", _("xwefax: Floating Point Exception") );
      exit(-1);

    case SIGABRT :
      fprintf( stderr, "%s\n", _("xwefax: Abort Signal received") );
      exit(-1);

    case SIGTERM :
      fprintf( stderr, "%s\n", _("xwefax: Termination Request received") );
      exit(-1);
  }

} /* End of sig_handler() */

/*------------------------------------------------------------------------*/

