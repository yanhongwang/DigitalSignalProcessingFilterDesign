#include <gnome.h>

union get_plu
{
	gchar seg[ 136 ];
};


gboolean
on_print_scale_delete_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_print_scale_show                    (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_login_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_logout_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_exit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_optionmenu1_changed                 (GtkOptionMenu   *optionmenu,
                                        gpointer         user_data);

void
on_optionmenu2_changed                 (GtkOptionMenu   *optionmenu,
                                        gpointer         user_data);

gboolean
on_about_delete_event                  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_about_close_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_optionmenu3_changed                 (GtkOptionMenu   *optionmenu,
                                        gpointer         user_data);

