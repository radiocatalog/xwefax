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

#include "interface.h"
#include "shared.h"

/*------------------------------------------------------------------*/

/* Gtk_Builder()
 *
 * Returns a GtkBuilder with required objects from file
 */
  static void
Gtk_Builder( GtkBuilder **builder, gchar **object_ids )
{
  GError *gerror = NULL;
  int ret = 0;

  /* Create a builder from object ids */
  *builder = gtk_builder_new();
  ret = (int)gtk_builder_add_objects_from_file(
      *builder, rc_data.xwefax_glade, object_ids, &gerror );
  if( !ret )
  {
    fprintf( stderr, "Xwefax: failed to add objects to builder:\n%s\n",
        gerror->message );

    fprintf( stderr, "Xwefax: Listing Object Ids:\n" );
    int idx = 0;
    while( object_ids[idx] != NULL )
    {
      fprintf( stderr, "%s\n", object_ids[idx] );
      idx++;
    }

    exit( -1 );
  }

  /* Connect signals if gmodule is supported */
  if( !g_module_supported() )
  {
    fprintf( stderr, "Xwefax: lib gmodule not supported\n" );
    exit( -1 );
  }
  gtk_builder_connect_signals( *builder, NULL );

} /* Gtk_Builder() */

/*------------------------------------------------------------------*/

/* Builder_Get_Object()
 *
 * Gets a named object from the GtkBuilder builder object
 */
  GtkWidget *
Builder_Get_Object( GtkBuilder *builder, gchar *name )
{
  GObject *object = gtk_builder_get_object( builder, name );
  if( object == NULL )
  {
    fprintf( stderr,
        "!! Xwefax: builder failed to get named object: %s\n", name );
    exit( -1 );
  }

  return( GTK_WIDGET(object) );
} /* Builder_Get_Object() */

/*------------------------------------------------------------------*/

  GtkWidget *
create_main_window( GtkBuilder **builder )
{
  gchar *object_ids[] = { MAIN_WINDOW_IDS };
  Gtk_Builder( builder, object_ids );
  GtkWidget *window = Builder_Get_Object( *builder, "main_window" );
  return( window );
}

GtkWidget *
create_error_dialog( GtkBuilder **builder )
{
  gchar *object_ids[] = { ERROR_DIALOG_IDS };
  Gtk_Builder( builder, object_ids );
  GtkWidget *dialog = Builder_Get_Object( *builder, "error_dialog" );
  return( dialog );
}

  GtkWidget *
create_quit_dialog( GtkBuilder **builder )
{
  gchar *object_ids[] = { QUIT_DIALOG_IDS };
  Gtk_Builder( builder, object_ids );
  GtkWidget *dialog = Builder_Get_Object( *builder, "quit_dialog" );
  return( dialog );
}

  GtkWidget *
create_popup_menu( GtkBuilder **builder )
{
  gchar *object_ids[] = { POPUP_MENU_IDS };
  Gtk_Builder( builder, object_ids );
  GtkWidget *menu = Builder_Get_Object( *builder, "popup_menu" );
  return( menu );
}

  GtkWidget *
create_stations_window( GtkBuilder **builder )
{
  gchar *object_ids[] = { STATIONS_WINDOW_IDS };
  Gtk_Builder( builder, object_ids );
  GtkWidget *window = Builder_Get_Object( *builder, "stations_window" );
  return( window );
}

