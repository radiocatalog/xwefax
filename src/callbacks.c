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

#include "callbacks.h"
#include "interface.h"
#include "shared.h"

/* File variables */
static GtkWidget
  *quit_dialog  = NULL,
  *error_dialog = NULL;


/*  Error_Dialog()
 *
 *  Opens an error dialog box
 */
  void
Error_Dialog( char *mesg, gboolean hide )
{
  GtkBuilder *builder;
  if( !error_dialog )
  {
    error_dialog = create_error_dialog( &builder );
    GtkWidget *lbl = Builder_Get_Object( builder, "error_message" );
    gtk_label_set_text( GTK_LABEL(lbl), mesg );
    if( hide ) gtk_widget_hide( Builder_Get_Object(builder, "error_ok_button") );
    gtk_widget_show( error_dialog );
    g_object_unref( builder );
  }
}


  gboolean
on_main_window_delete_event(
    GtkWidget *widget,
    GdkEvent  *event,
    gpointer   user_data)
{
  GtkBuilder *builder;
  if( !quit_dialog )
  {
    quit_dialog = create_quit_dialog( &builder );
    gtk_widget_show( quit_dialog );
    g_object_unref( builder );
  }
//  SetFlag( RECEIVE_STOP );
//  SetFlag( XWEFAX_QUIT );
  return( TRUE );
}


  void
on_main_window_destroy(
    GObject  *object,
    gpointer  user_data)
{
  SetFlag( RECEIVE_STOP );
  SetFlag( XWEFAX_QUIT );
  Cleanup();
  gtk_main_quit();
}


  void
on_scope_drawingarea_configure_event(
    GtkWidget       *widget,
    GdkEventConfigure *event,
    gpointer         user_data)
{
  scope_width  = event->width;
  scope_height = event->height;
}

  void
on_spectrum_drawingarea_configure_event(
    GtkWidget         *widget,
    GdkEventConfigure *event,
    gpointer           user_data)
{
  Spectrum_Size_Allocate( event->width, event->height );
}


  gboolean
on_spectrum_drawingarea_draw(
    GtkWidget *widget,
    cairo_t   *cr,
    gpointer   user_data)
{
  if( wfall_pixbuf != NULL )
  {
    gdk_cairo_set_source_pixbuf( cr, wfall_pixbuf, 0.0, 0.0 );
    cairo_paint( cr );
    return( TRUE );
  }
  return( FALSE );
}


  gboolean
on_spectrum_button_press_event(
    GtkWidget      *widget,
    GdkEventButton *event,
    gpointer        user_data)
{
  Tune_Tcvr( event->x );
  return( FALSE );
}


  gboolean
on_error_dialog_delete_event(
    GtkWidget *widget,
    GdkEvent   *event,
    gpointer    user_data)
{
  return( TRUE );
}


  void
on_error_dialog_destroy(
    GObject  *object,
    gpointer  user_data)
{
  error_dialog = NULL;
}


  void
on_quit_dialog_destroy(
    GObject  *object,
    gpointer  user_data)
{
  quit_dialog = NULL;
}


  void
on_error_ok_button_clicked(
    GtkButton *button,
    gpointer   user_data)
{
  gtk_widget_destroy( error_dialog );
}


  void
on_error_quit_button_clicked(
    GtkButton *button,
    gpointer   user_data)
{
  SetFlag( RECEIVE_STOP );
  SetFlag(XWEFAX_QUIT);
  Cleanup();
  gtk_main_quit();
}


  void
on_quit_cancel_button_clicked(
    GtkButton *button,
    gpointer   user_data)
{
  ClearFlag(XWEFAX_QUIT);
  gtk_widget_destroy( quit_dialog );
}


  void
on_quit_button_clicked(
    GtkButton *button,
    gpointer   user_data)
{
  gtk_widget_destroy( main_window );
}


  gboolean
on_scope_drawingarea_draw(
    GtkWidget *widget,
    cairo_t   *cr,
    gpointer   user_data)
{
  /* Draw scope background */
  if( isFlagClear(ENABLE_SCOPE) )
  {
    cairo_set_source_rgb( cr, SCOPE_BACKGND );
    cairo_rectangle( cr, 0.0, 0.0, (double)scope_width, (double)scope_height );
    cairo_fill( cr );
  }
  else Draw_Signal( cr );
  return( TRUE );
}


  void
on_start_togglebutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  Start_Button_Toggled( togglebutton );
}


  void
on_skip_button_clicked(
    GtkButton *button,
    gpointer   user_data)
{
  SetFlag( SKIP_ACTION );
}


  void
on_quit_activate(
    GtkMenuItem *menuitem,
    gpointer     user_data)
{
  GtkBuilder *builder;
  if( !quit_dialog )
  {
    quit_dialog = create_quit_dialog( &builder );
    gtk_widget_show( quit_dialog );
    g_object_unref( builder );
  }
}


  void
on_rpm_activate(
    GtkMenuItem *menuitem,
    gpointer     user_data)
{
  New_Lines_Per_Min();
}


  void
on_pix_activate(
    GtkMenuItem *menuitem,
    gpointer     user_data)
{
  New_Pixels_Per_Line();
}


  void
on_ioc_activate(
    GtkMenuItem *menuitem,
    gpointer     user_data)
{
  New_IOC();
}


  void
on_phl_activate(
    GtkMenuItem *menuitem,
    gpointer     user_data)
{
  New_Phasing_Lines();
}


  void
on_enhance_activate(
    GtkMenuItem *menuitem,
    gpointer     user_data)
{
  New_Image_Enhance();
}


  void
on_deslant_spinbutton_value_changed(
    GtkSpinButton *spinbutton,
    gpointer       user_data)
{
  double sync_slant = gtk_spin_button_get_value( spinbutton );
  Set_Sync_Slant( sync_slant );
}


  void
on_in_image_phasing_activate(
    GtkMenuItem *menuitem,
    gpointer     user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
    SetFlag( INIMAGE_PHASING );
  else
    ClearFlag( INIMAGE_PHASING );
}


  gboolean
on_wefax_drawingarea_button_press_event(
    GtkWidget      *widget,
    GdkEventButton  *event,
    gpointer         user_data)
{
  return( Wefax_Drawingarea_Button_Press(event) );
}


  gboolean
on_wefax_drawingarea_draw(
    GtkWidget *widget,
    cairo_t   *cr,
    gpointer   user_data)
{
  if( wefax_pixbuf != NULL )
  {
    gdk_cairo_set_source_pixbuf( cr, wefax_pixbuf, 0.0, 0.0 );
    cairo_paint( cr );
    return( TRUE );
  }
  return( FALSE );
}


  void
on_station_list_activate(
    GtkMenuItem *menuitem,
    gpointer     user_data)
{
  if( stations_window == NULL ) List_Stations();
}


  void
on_stations_window_destroy(
    GObject  *object,
    gpointer  user_data)
{
  ClearFlag( STATIONS_LIST_OK );
  stations_list_store = NULL;
  stations_treeview   = NULL;
  stations_window     = NULL;
  if( isFlagSet(CAT_SETUP) ) Close_Tcvr_Serial();
}


  void
on_down_button_clicked(
    GtkButton *button,
    gpointer   user_data)
{
  Select_Treeview_Row( SELECT_ROW_DOWN );
}


  void
on_up_button_clicked(
    GtkButton *button,
    gpointer   user_data)
{
  Select_Treeview_Row( SELECT_ROW_UP );
}


  void
on_save_button_clicked(
    GtkButton *button,
    gpointer   user_data)
{
  SetFlag( SAVE_STATIONS );
  if( wefax_action == ACTION_STOP )
    Save_Stations_File( rc_data.stations_file );
}


  void
on_delete_button_clicked(
    GtkButton *button,
    gpointer   user_data)
{
  Delete_Station_Row();
}


  void
on_new_button_clicked(
    GtkButton *button,
    gpointer   user_data)
{
  New_Station_Row();
}


  void
on_stations_treeview_cursor_changed(
    GtkTreeView *treeview,
    gpointer     user_data)
{
  Stations_Cursor_Changed( treeview );
}


  gboolean
on_stations_treeview_button_press_event(
    GtkWidget      *widget,
    GdkEventButton *event,
    gpointer        user_data)
{
  if( event->button == 2 )
  {
    Treeview_Button_Press();
    return( TRUE );
  }
  return( FALSE );
}


  void
on_capture_setup_activate(
    GtkMenuItem *menuitem,
    gpointer     user_data)
{
  GtkLabel *lbl = GTK_LABEL(
      Builder_Get_Object(main_window_builder, "scope_label") );
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
  {
    ClearFlag( DISPLAY_SIGNAL );
    gtk_label_set_text( lbl, "Capture Setup" );
  }
  else
  {
    SetFlag( DISPLAY_SIGNAL );
    gtk_label_set_text( lbl, "Signal" );
  }
}


  void
on_jpeg_activate(
    GtkMenuItem *menuitem,
    gpointer     user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
  {
    SetFlag( SAVE_IMAGE_JPG );
    ClearFlag( SAVE_IMAGE_PGM );
  }
}


  void
on_pgm_activate(
    GtkMenuItem *menuitem,
    gpointer     user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
  {
    SetFlag( SAVE_IMAGE_PGM );
    ClearFlag( SAVE_IMAGE_JPG );
  }
}


  void
on_both_activate(
    GtkMenuItem *menuitem,
    gpointer     user_data)
{
  if( gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )
  {
    SetFlag( SAVE_IMAGE_JPG );
    SetFlag( SAVE_IMAGE_PGM );
  }
}


  void
on_zerocrossing_activate(
    GtkMenuItem *menuitem,
    gpointer     user_data)
{
  FM_Detector = FM_Detect_Zero_Crossing;
}


  void
on_bilevel_activate(
    GtkMenuItem *menuitem,
    gpointer     user_data)
{
  FM_Detector = FM_Detect_Bilevel;
}


  gboolean
on_gauge_drawingarea_draw(
    GtkWidget *widget,
    cairo_t   *cr,
    gpointer   user_data)
{
  Display_Level_Gauge( cr );
  return( TRUE );
}


  void
on_save_checkbutton_toggled(
    GtkToggleButton *togglebutton,
    gpointer         user_data)
{
  if( gtk_toggle_button_get_active(togglebutton) )
  {
    Set_Indicators( ICON_SAVE_YES );
    SetFlag( SAVE_IMAGE );
  }
  else
  {
    Set_Indicators( ICON_SAVE_SKIP);
    ClearFlag( SAVE_IMAGE );
  }
}

