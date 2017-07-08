#include <gtk/gtk.h>


gboolean
on_main_window_delete_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_main_window_destroy                 (GtkObject       *object,
                                        gpointer         user_data);

void
on_scope_size_allocate                 (GtkWidget       *widget,
                                        GdkRectangle    *allocation,
                                        gpointer         user_data);

gboolean
on_scope_expose_event                  (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

gboolean
on_spectrum_button_press_event         (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

void
on_spectrum_size_allocate              (GtkWidget       *widget,
                                        GdkRectangle    *allocation,
                                        gpointer         user_data);

gboolean
on_spectrum_expose_event               (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

void
on_start_togglebutton_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_skip_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_quit_cancel_button_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_quit_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_rpm_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_pix_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_ioc_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_phl_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_in_image_phasing_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_quit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_error_dialog_destroy                (GtkObject       *object,
                                        gpointer         user_data);

gboolean
on_error_dialog_delete_event           (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_error_quit_button_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_error_ok_button_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_enhance_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_wefax_drawingarea_button_press_event
                                        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_wefax_drawingarea_expose_event      (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);
void
on_station_window_destroy              (GtkObject       *object,
                                        gpointer         user_data);

void
on_station_list_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_stations_window_destroy             (GtkObject       *object,
                                        gpointer         user_data);


gboolean
on_stations_treeview_button_press_event
                                        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

void
on_stations_treeview_cursor_changed    (GtkTreeView     *treeview,
                                        gpointer         user_data);

void
on_down_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_up_button_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_save_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_delete_button_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_new_button_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_stations_treeview_button_press_event
                                        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

void
on_deslant_spinbutton_value_changed     (GtkSpinButton   *spinbutton,
                                        gpointer         user_data);

void
on_capture_setup_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_jpeg_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_pgm_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_both_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_zerocrossing_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_bilevel_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_control_drawingarea_expose_event    (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

gboolean
on_control_drawingarea_visibility_notify_event
                                        (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_control_drawingarea_expose_event    (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);


void
on_save_checkbutton_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

