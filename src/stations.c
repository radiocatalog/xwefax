/*  stations.c
 *
 *  Stations treview functions for xwefax application
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

#include "stations.h"

/*------------------------------------------------------------------------*/

/* Create_List_Stores()
 *
 * Create stores needed for the treeview
 */
  void
Create_List_Store( void )
{
  /* Station list column names */
  char *list_col_name[LIST_NUM_COLS] =
  {
	_("STATION NAME"),
	_("FREQUENCY"),
	_(" S/B"),
	_("RPM"),
	_("PIX/L"),
	_(" IOC"),
	_("PHL"),
	_("SLANT  ")
  };

  /* Create list stores only if needed */
  if( stations_list_store != NULL ) return;

  /* Create stations list store */
  stations_list_store = gtk_list_store_new(
	  LIST_NUM_COLS,
	  G_TYPE_STRING,
	  G_TYPE_STRING,
	  G_TYPE_STRING,
	  G_TYPE_STRING,
	  G_TYPE_STRING,
	  G_TYPE_STRING,
	  G_TYPE_STRING,
	  G_TYPE_STRING );

  /* Insert station list columns */
  Insert_Columns( stations_list_store, LIST_NUM_COLS, list_col_name );

} /* Create_List_Store() */

/*------------------------------------------------------------------------*/

/* Insert_Columns()
 *
 * Inserts columns in a list store
 */
  void
Insert_Columns(	GtkListStore* store, int ncols, char *colname[] )
{
  int idx;
  GtkTreeModel *model;
  GtkCellRenderer *renderer;

  /* Create renderer and insert columns to treeview */
  for( idx = 0; idx < ncols; idx++ )
  {
	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "editable", TRUE, NULL);
	g_signal_connect( renderer, "edited",
		(GCallback)cell_edited_callback, stations_treeview );
	g_object_set_data( G_OBJECT(renderer),
		"column", GUINT_TO_POINTER(idx) );
	gtk_tree_view_insert_column_with_attributes(
		stations_treeview, -1, colname[idx],
		renderer, "text", idx, NULL );
  }

  /* Create a model to insert list store */
  model = GTK_TREE_MODEL( store );
  gtk_tree_view_set_model( stations_treeview, model );

  /* Destroy model automatically with view */
  g_object_unref( model );

} /* Insert_Columns() */

/*------------------------------------------------------------------------*/

/* List_Stations()
 *
 * Reads stations data from file and lists in tree view
 */
  void
List_Stations( void )
{
  gboolean ret;			 /* Return status variable */
  FILE *fp;				 /* Stations file pointer */
  int eof;				 /* End of File flag */
  GtkTreeIter iter;		 /* Treeview list iteration */
  GtkTreeSelection *sel; /* Selected treeview row */
  GtkTreePath *path;	 /* Path to ead data from treeview */
  int start, end;		 /* Start and end character of strings in treeview */
  char line_buf[LINE_BUFF_LEN + 1]; /* Buffers one line of station data */

  /* Create treeview pointer */
  stations_treeview = GTK_TREE_VIEW(
	  lookup_widget(stations_window, "stations_treeview") );

  /* Create the list store */
  Create_List_Store();

  /* Open stations file */
  if( !Open_File(&fp, rc_data.stations_file, "r") )
  {
	Show_Message( "Failed to open Stations\n"
		"File in ~/xwefax", "red" );
	Error_Dialog( "Failed to open Stations\n"
		"File in ~/xwefax", OK );
	return;
  }

  /* Clear all tree view */
  gtk_list_store_clear( stations_list_store );

  /* Get new row if available */
  ret = gtk_tree_model_get_iter_first(
	  GTK_TREE_MODEL(stations_list_store), &iter);

  /* Fill station list */
  while( !ret )
  {
	/* Read a line and separate entries */
	eof = Load_Line( line_buf, fp, "Station File" );
	if( eof == ERROR )
	{
	  Show_Message( "Error reading Stations\n"
		  "File in ~/xwefax", "red" );
	  Error_Dialog( "Error reading Stations\n"
		  "File in ~/xwefax", OK );
	  return;
	}

	/* Append a row and fill in station data */
	gtk_list_store_append( stations_list_store, &iter );

	/* Separate entries in the line by string
	 * terminator and set data to list store */
	/* Station Name */
	start = 0;
	end = STATIONS_NAME_WIDTH;
	line_buf[ end ] = '\0';
	gtk_list_store_set( stations_list_store,
		&iter, NAME_COL, &line_buf[start], -1 );

	/* Frequency - Hz */
	end += STATIONS_FREQ_WIDTH + 1;
	line_buf[ end ] = '\0';
	start += STATIONS_NAME_WIDTH + 1;
	gtk_list_store_set( stations_list_store,
		&iter, FREQ_COL, &line_buf[start], -1 );

	/* Sideband - USB or LSB */
	end += STATIONS_SDB_WIDTH + 1;
	line_buf[ end ] = '\0';
	start += STATIONS_FREQ_WIDTH + 1;
	gtk_list_store_set( stations_list_store,
		&iter, SDB_COL, &line_buf[start], -1 );

	/* RPM - lines/min */
	end += STATIONS_RPM_WIDTH + 1;
	line_buf[ end ] = '\0';
	start += STATIONS_SDB_WIDTH + 1;
	gtk_list_store_set( stations_list_store,
		&iter, RPM_COL, &line_buf[start], -1 );

	/* Resolution - pix/line */
	end += STATIONS_RESOL_WIDTH + 1;
	line_buf[ end ] = '\0';
	start += STATIONS_RPM_WIDTH + 1;
	gtk_list_store_set( stations_list_store,
		&iter, RESOL_COL, &line_buf[start], -1 );

	/* IOC Value */
	end += STATIONS_IOC_WIDTH + 1;
	line_buf[ end ] = '\0';
	start += STATIONS_RESOL_WIDTH + 1;
	gtk_list_store_set( stations_list_store,
		&iter, IOC_COL, &line_buf[start], -1 );

	/* Phasing Lines Value */
	end += STATIONS_PHL_WIDTH + 1;
	line_buf[ end ] = '\0';
	start += STATIONS_IOC_WIDTH + 1;
	gtk_list_store_set( stations_list_store,
		&iter, PHL_COL, &line_buf[start], -1 );

	/* Slant Value */
	end += STATIONS_SLANT_WIDTH + 1;
	line_buf[ end ] = '\0';
	start += STATIONS_PHL_WIDTH + 1;
	gtk_list_store_set( stations_list_store,
		&iter, SLANT_COL, &line_buf[start], -1 );

	/* Stop at end of file */
	if( eof == EOF ) break;

	/* Get new row if available */
	ret = gtk_tree_model_iter_next(
		GTK_TREE_MODEL(stations_list_store), &iter);

  } /* while() */

  /* Select the first entry */
  sel = gtk_tree_view_get_selection( stations_treeview );
  path = gtk_tree_path_new_from_string( "0" );
  gtk_tree_selection_select_path( sel, path );
  gtk_tree_path_free( path );

  ClearFlag( SAVE_STATIONS );
  fclose( fp );

} /* List_Stations() */

/*------------------------------------------------------------------------*/

/* Save_Stations_File()
 *
 * Saves the data in stations treeview to a given filename
 */
  gboolean
Save_Stations_File( char *stations_file )
{
  FILE *fp = NULL;		/* Stations file pointer */
  GtkTreeIter iter;		/* Iteration to station treeview */
  char file_line[STATIONS_LINE_LEN]; /* Buffer for one file line */
  gboolean ret = FALSE;	/* Return value of functions */
  char *string;			/* String created from Wefax parameters */
  size_t len, limit;	/* Length and its limit of above string */
  int idx, value;
  char str[12];

  /* Do not save if treeview is not
   * valid or save flag not set */
  if( (stations_treeview == NULL) || isFlagClear(SAVE_STATIONS) )
	return( FALSE );

  /* Stations treeview model */
  GtkTreeModel *model = GTK_TREE_MODEL(stations_list_store);

  /* Get the first iteration */
  ret = gtk_tree_model_get_iter_first( model, &iter);

  /* Open stations file for writing */
  if( ret && !Open_File(&fp, (char *)stations_file, "w") )
  {
	Show_Message( "Failed to open Stations\n"
		"File in ~/xwefax", "red" );
	Error_Dialog( "Failed to open Stations\n"
		"File in ~/xwefax", OK );
	return( FALSE );
  }

  /* Write treeview data */
  Show_Message( "Saving Stations File ...", "black" );
  while( ret )
  {
	/* Clear file line */
	for( idx = 0; idx < STATIONS_LINE_LEN; idx++ )
	  file_line[idx] = ' ';

	/* Get the station name */
	gtk_tree_model_get( model, &iter, NAME_COL, &string, -1 );
	len = strlen( string );
	limit = STATIONS_NAME_WIDTH;
	if( len > limit ) len = limit;

	/* Remove leading spaces */
	idx = 0;
	while( (string[idx] == ' ') && (idx < (int)len) )
	{
	  idx++;
	  len--;
	}
	strncpy( file_line, &string[idx], len );
	idx = STATIONS_NAME_WIDTH + 1;
	g_free( string );

	/* Get the station frequency */
	gtk_tree_model_get( model, &iter, FREQ_COL, &string, -1 );
	value = atoi( string );

	/* Format value before writing to file */
	snprintf( str, STATIONS_FREQ_WIDTH + 1, "%11d", value );
	strncpy( &file_line[idx], str, STATIONS_FREQ_WIDTH );
	g_free( string );
	idx += STATIONS_FREQ_WIDTH + 1;

	/* Get and write the station Sideband */
	gtk_tree_model_get( model, &iter, SDB_COL, &string, -1 );
	strncpy( &file_line[idx], string, STATIONS_SDB_WIDTH );
	g_free( string );
	idx += STATIONS_SDB_WIDTH + 1;

	/* Get the station RPM */
	gtk_tree_model_get( model, &iter, RPM_COL, &string, -1 );
	value = atoi( string );

	/* Format value before writing to file */
	snprintf( str, STATIONS_RPM_WIDTH + 1, "%4d", value );
	strncpy( &file_line[idx], str, STATIONS_RPM_WIDTH );
	g_free( string );
	idx += STATIONS_RPM_WIDTH + 1;

	/* Get the station resolution */
	gtk_tree_model_get( model, &iter, RESOL_COL, &string, -1 );
	value = atoi( string );

	/* Format value before writing to file */
	snprintf( str, STATIONS_RESOL_WIDTH + 1, "%5d", value );
	strncpy( &file_line[idx], str, STATIONS_RESOL_WIDTH );
	g_free( string );
	idx += STATIONS_RESOL_WIDTH + 1;

	/* Get the station ioc */
	gtk_tree_model_get( model, &iter, IOC_COL, &string, -1 );
	value = atoi( string );

	/* Format value before writing to file */
	snprintf( str, STATIONS_IOC_WIDTH + 1, "%4d", value );
	strncpy( &file_line[idx], str, STATIONS_IOC_WIDTH );
	g_free( string );
	idx += STATIONS_IOC_WIDTH + 1;

	/* Get the station phasing lines */
	gtk_tree_model_get( model, &iter, PHL_COL, &string, -1 );
	value = atoi( string );

	/* Format value before writing to file */
	snprintf( str, STATIONS_PHL_WIDTH + 1, "%3d", value );
	strncpy( &file_line[idx], str, STATIONS_PHL_WIDTH );
	g_free( string );
	idx += STATIONS_PHL_WIDTH + 1;

	/* Get the station sync slant */
	gtk_tree_model_get( model, &iter, SLANT_COL, &string, -1 );
	value = atoi( string );

	/* Format value before writing to file */
	snprintf( str, STATIONS_SLANT_WIDTH + 1, "%4d", value );
	strncpy( &file_line[idx], str, STATIONS_SLANT_WIDTH );
	g_free( string );

	/* Write line to stations file */
	file_line[STATIONS_LINE_LEN - 1] = '\0';
	fprintf( fp, "%s\n", file_line );

	ret = gtk_tree_model_iter_next( model, &iter);
  } /* while( ret ) */

  Show_Message( "Stations File saved OK", "green" );
  ClearFlag( SAVE_STATIONS );
  fclose( fp );

  return( FALSE );
} /* Save_Stations_File() */

/*------------------------------------------------------------------------*/

/* Select_Treeview_Row()
 *
 * Selects a stations treeview row on user click
 * on up or down buttons in stations window
 */
  void
Select_Treeview_Row( int direction )
{
  static int row_num = 0;
  char path_str[3];		 /* String to create a path from */
  GtkTreeSelection *sel; /* Selected row in treeview */
  GtkTreePath *path;	 /* A path into the selected row */

  /* Increment or decrement row number
   * and clamp between 0 and 99 */
  if( direction == SELECT_ROW_DOWN )
  {
	row_num ++;
	if( row_num > 99 ) row_num = 99;
  }
  else if( direction == SELECT_ROW_UP )
  {
	row_num--;
	if( row_num < 0 ) row_num = 0;
  }

  /* Create a selection object */
  sel = gtk_tree_view_get_selection( stations_treeview );

  /* Create a string for new path and select row */
  snprintf( path_str, sizeof(path_str), "%d", row_num );
  path = gtk_tree_path_new_from_string( path_str );
  gtk_tree_selection_select_path( sel, path );
  gtk_tree_path_free( path );

  /* Simulate a button press on treeview */
  Stations_Cursor_Changed( stations_treeview );

} /* Select_Treeview_Row() */

/*------------------------------------------------------------------------*/

/* Stations_Cursor_Changed()
 *
 * Handles cursor changed event on stations treeview.
 * Reads the selected row and sets menu items with the
 * new values of wefax parameters and tunes the receiver
 */
  void
Stations_Cursor_Changed( GtkTreeView *treeview )
{
  /* Stations treeview selection objects */
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter;

  GtkLabel *label;	/* Set the Wefax image label to station name */
  gchar *name;		/* Station name */
  static gchar current[STATIONS_NAME_WIDTH + 1]; /* Saves above name */

  gchar *value;		/* Value from treeview strings */
  int idx, len, slant;


  /* Get the selected row */
  selection = gtk_tree_view_get_selection( treeview );
  gtk_tree_selection_get_selected( selection, &model, &iter );

  /* Get the station name */
  gtk_tree_model_get( model, &iter, NAME_COL, &name, -1 );

  /* If stations row has not changed
   * or name not valid, do nothing */
  current[STATIONS_NAME_WIDTH] = '\0';
  if( (strcmp(current, name) == 0) || (strstr(name, "--")) )
  {
	g_free( name );
	return;
  }

  /* Save current station name */
  Strlcpy( current, name, sizeof(current) );

  /* Remove trailing spaces */
  len = (int)strlen( name ) - 1;
  for( idx = len; idx >= 0; idx-- )
	if( name[idx] != ' ' ) break;
  name[ idx + 1 ] = '\0';

  /* Set the label of wefax image frame */
  label = GTK_LABEL( lookup_widget(main_window, "image_label") );
  gtk_label_set_text( label, name );
  g_free( name );

  /* Get the RX frequency */
  gtk_tree_model_get( model, &iter, FREQ_COL, &value, -1 );
  if( strstr(value, "--") ) return;
  rc_data.station_freq = atoi( value );
  g_free( value );

  /* Get the station Sideband */
  gtk_tree_model_get( model, &iter, SDB_COL, &value, -1 );
  if( strstr(value, "--") ) return;
  if( strstr(value, "LSB") )
	strncpy( rc_data.station_sideband, "LSB",
		sizeof(rc_data.station_sideband) );
  else
	strncpy( rc_data.station_sideband, "USB",
		sizeof(rc_data.station_sideband) );
  g_free( value );

  /* Get the RPM */
  gtk_tree_model_get( model, &iter, RPM_COL, &value, -1 );
  if( strstr(value, "--") ) return;
  rc_data.lines_per_min = atof( value );
  g_free( value );

  /* Get the resolution */
  gtk_tree_model_get( model, &iter, RESOL_COL, &value, -1 );
  if( strstr(value, "--") ) return;
  rc_data.pixels_per_line = atoi( value );
  g_free( value );

  /* Get the IOC */
  gtk_tree_model_get( model, &iter, IOC_COL, &value, -1 );
  if( strstr(value, "--") ) return;
  rc_data.ioc_value = atoi( value );
  g_free( value );

  /* Period of start and stop tones in pixels */
  if( rc_data.ioc_value == IOC576 )
	rc_data.start_tone = IOC576_START_TONE;
  else if( rc_data.ioc_value == IOC288 )
	rc_data.start_tone = IOC288_START_TONE;
  double temp = rc_data.lines_per_min / 60.0; /* lines/sec */
  rc_data.start_tone_period =
	temp * (double)rc_data.pixels_per_line / (double)rc_data.start_tone;
  rc_data.stop_tone_period =
	temp * (double)rc_data.pixels_per_line / (double)WEFAX_STOP_TONE;

  /* Get the phasing lines */
  gtk_tree_model_get( model, &iter, PHL_COL, &value, -1 );
  if( strstr(value, "--") ) return;
  rc_data.phasing_lines = atoi( value );
  g_free( value );

  /* Get the slant and set spin button */
  gtk_tree_model_get( model, &iter, SLANT_COL, &value, -1 );
  if( strstr(value, "--") ) return;
  slant = atoi( value );
  g_free( value );
  GtkSpinButton *spinb = GTK_SPIN_BUTTON(
	  lookup_widget(main_window, "deslant_spinbutton") );
  gtk_spin_button_set_value( spinb, slant );

  /* Configure xwefax for new station */
  Configure();

  /* Set Rx freq and mode */
 gtk_idle_add( Set_Rx_Freq_Idle_Cb, NULL );

} /* Stations_Cursor_Changed() */

/*------------------------------------------------------------------------*/

/* Treeview_Button_Press()
 *
 * Handles the stations treeview button press callback
 */
  void
Treeview_Button_Press( void )
{
  /* Stations treeview selection objects */
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter iter;
  int sync_slant;

  /* Data from stations treeview */
  gchar
	freq[STATIONS_FREQ_WIDTH+1],
	sdb[STATIONS_SDB_WIDTH+1],
	rpm[STATIONS_RPM_WIDTH+1],
	resol[STATIONS_RESOL_WIDTH+1],
	ioc[STATIONS_IOC_WIDTH+1],
	phl[STATIONS_PHL_WIDTH+1],
	slant[STATIONS_SLANT_WIDTH+1];

  /* Get the selected row */
  selection = gtk_tree_view_get_selection( stations_treeview );
  gtk_tree_selection_get_selected( selection, &model, &iter );

  /* Create data to insert to treeview */
  if( rc_data.tcvr_type != PERSEUS )
	Read_Rx_Freq( &(rc_data.station_freq) );

  /* Compensate slant correction for Perseus Rx */
  sync_slant = (int)( rc_data.sync_slant * 1000.0 );
  if( rc_data.tcvr_type == PERSEUS )
	sync_slant -= rc_data.perseus_rate_correction;

  snprintf( freq,  sizeof(freq),  "%*d",
	  STATIONS_FREQ_WIDTH, rc_data.station_freq );
  snprintf( sdb,  sizeof(sdb),  "%*s",
	  STATIONS_SDB_WIDTH, rc_data.station_sideband );
  snprintf( rpm,   sizeof(rpm),   "%*d",
	  STATIONS_RPM_WIDTH, (int)rc_data.lines_per_min );
  snprintf( resol, sizeof(resol), "%*d",
	  STATIONS_RESOL_WIDTH, rc_data.pixels_per_line );
  snprintf( ioc,   sizeof(ioc),   "%*d",
	  STATIONS_IOC_WIDTH, rc_data.ioc_value );
  snprintf( phl,   sizeof(phl),   "%*d",
	  STATIONS_PHL_WIDTH, rc_data.phasing_lines );
  snprintf( slant, sizeof(slant), "%*d",
	  STATIONS_SLANT_WIDTH, sync_slant );

  /* Set data to list store */
  gtk_list_store_set(
	  stations_list_store, &iter,
	  FREQ_COL,  freq,
	  SDB_COL,   sdb,
	  RPM_COL,   rpm,
	  RESOL_COL, resol,
	  IOC_COL,   ioc,
	  PHL_COL,   phl,
	  SLANT_COL, slant,
	  -1 );

} /* Treeview_Button_Press() */

/*------------------------------------------------------------------------*/

/* New_Station_Row()
 *
 * Adds a new row to the stations treeview
 */
  void
New_Station_Row( void )
{
  /* Stations treeview selection objects */
  GtkTreeModel *model;
  GtkTreeSelection *selection;
  GtkTreeIter iter, sibling;
  int ncols, idx;

  if( stations_treeview == NULL )
	return;

  /* Find selected row and add new after */
  selection = gtk_tree_view_get_selection( stations_treeview );
  if( ! gtk_tree_selection_get_selected(selection, &model, &sibling) )
  {
	/* Empty tree view case */
	model = gtk_tree_view_get_model( stations_treeview );
	gtk_list_store_insert( GTK_LIST_STORE(model), &iter, 0 );
  }
  else gtk_list_store_insert_after(
	  GTK_LIST_STORE(model), &iter, &sibling);

  /* Prime columns of new row */
  gtk_tree_selection_select_iter( selection, &iter);
  ncols = gtk_tree_model_get_n_columns( model );
  for( idx = 0; idx < ncols; idx++ )
	gtk_list_store_set(
		GTK_LIST_STORE(model), &iter, idx, "--", -1 );

} /* New_Station_Row() */

/*------------------------------------------------------------------------*/

/* Delete_Station_Row()
 *
 * Deletes a selected row in the stations treeview
 */
  void
Delete_Station_Row( void )
{
  /* Stations treeview selection objects */
  GtkTreeModel *model;
  GtkTreeIter   iter;
  GtkTreeSelection *selection;

  if( stations_treeview == NULL )
    return;

  selection = gtk_tree_view_get_selection( stations_treeview );
  gtk_tree_selection_get_selected( selection, &model, &iter);
  gtk_list_store_remove( GTK_LIST_STORE(model), &iter );

} /* Delete_Station_Row() */

/*------------------------------------------------------------------------*/

/* cell_edited_callback()
 *
 * Text cell edited callback
 */
  void
cell_edited_callback(
	GtkCellRendererText *cell,
	gchar				*path,
	gchar               *new_text,
	gpointer             user_data )
{
  /* Stations treeview selection objects */
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GtkTreeIter       iter;
  guint column;

  column = GPOINTER_TO_UINT(
	  g_object_get_data(G_OBJECT(cell), "column") );
  selection = gtk_tree_view_get_selection(
	  GTK_TREE_VIEW(user_data) );
  gtk_tree_selection_get_selected(
	  selection, &model, &iter );
	gtk_list_store_set( GTK_LIST_STORE(model),
	  &iter, column, new_text, -1 );

} /* cell_edited_callback() */

/*------------------------------------------------------------------------*/

/* Helper function
  gboolean
gtk_tree_model_iter_previous(GtkTreeModel *tree_model, GtkTreeIter *iter)
{
  GtkTreePath *path;
  gboolean ret;

  path = gtk_tree_model_get_path (tree_model, iter);
  ret = gtk_tree_path_prev (path);
  if (ret == TRUE)
	gtk_tree_model_get_iter (tree_model, iter, path);
  gtk_tree_path_free (path);
  return ret;
} */

/*------------------------------------------------------------------------*/

