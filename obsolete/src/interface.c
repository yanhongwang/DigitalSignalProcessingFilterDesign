/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <bonobo.h>
#include <gnome.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  gtk_object_set_data_full (GTK_OBJECT (component), name, \
    gtk_widget_ref (widget), (GtkDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  gtk_object_set_data (GTK_OBJECT (component), name, widget)

static GnomeUIInfo menuitem1_menu_uiinfo[] =
{
  {
    GNOME_APP_UI_ITEM, N_("\347\231\273\345\205\245"),
    NULL,
    (gpointer) on_login_activate, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("\347\231\273\345\207\272"),
    NULL,
    (gpointer) on_logout_activate, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  GNOMEUIINFO_MENU_EXIT_ITEM (on_exit_activate, NULL),
  GNOMEUIINFO_END
};

static GnomeUIInfo menuitem4_menu_uiinfo[] =
{
  {
    GNOME_APP_UI_ITEM, N_("\351\227\234\346\226\274"),
    NULL,
    (gpointer) on_about_activate, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  GNOMEUIINFO_END
};

static GnomeUIInfo menubar1_uiinfo[] =
{
  {
    GNOME_APP_UI_SUBTREE, N_("\344\270\273\350\246\201"),
    NULL,
    menuitem1_menu_uiinfo, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  {
    GNOME_APP_UI_SUBTREE, N_("\345\271\253\345\212\251"),
    NULL,
    menuitem4_menu_uiinfo, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  GNOMEUIINFO_END
};

static GnomeUIInfo menu1_uiinfo[] =
{
  {
    GNOME_APP_UI_ITEM, N_("\351\207\215\351\207\217"),
    NULL,
    (gpointer) NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("\345\226\256\345\203\271"),
    NULL,
    (gpointer) NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("\344\270\262\345\210\227\345\237\240"),
    NULL,
    (gpointer) NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("\347\250\213\345\274\217\347\211\210\350\231\237"),
    NULL,
    (gpointer) NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("\346\216\247\345\210\266"),
    NULL,
    (gpointer) NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("\347\233\256\345\211\215\345\226\256\344\275\215"),
    NULL,
    (gpointer) NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("\345\256\271\345\231\250\351\207\215"),
    NULL,
    (gpointer) NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("\347\233\256\345\211\215\347\231\273\345\205\245\350\200\205"),
    NULL,
    (gpointer) NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("\346\231\202\351\226\223\346\227\245\346\234\237"),
    NULL,
    (gpointer) NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("\345\255\227\344\270\262\351\225\267\345\272\246"),
    NULL,
    (gpointer) NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  GNOMEUIINFO_END
};

static GnomeUIInfo menu3_uiinfo[] =
{
  {
    GNOME_APP_UI_ITEM, N_("\350\263\207\346\226\231\346\240\274\345\274\217\351\225\267\345\272\246"),
    NULL,
    (gpointer) NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("plu"),
    NULL,
    (gpointer) NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("\346\250\231\347\261\244"),
    NULL,
    (gpointer) NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("\345\255\227\344\270\262"),
    NULL,
    (gpointer) NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  GNOMEUIINFO_END
};

static GnomeUIInfo menu2_uiinfo[] =
{
  {
    GNOME_APP_UI_ITEM, N_("\345\256\271\351\207\217\351\207\215"),
    NULL,
    (gpointer) NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("\346\242\235\347\242\274\345\210\227\345\215\260\346\240\274\345\274\217"),
    NULL,
    (gpointer) NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("\345\255\227\344\270\262"),
    NULL,
    (gpointer) NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("\346\267\273\345\212\240\345\255\227\344\270\262"),
    NULL,
    (gpointer) NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  {
    GNOME_APP_UI_ITEM, N_("plu"),
    NULL,
    (gpointer) NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL,
    0, (GdkModifierType) 0, NULL
  },
  GNOMEUIINFO_END
};

GtkWidget*
create_print_scale (void)
{
  GtkWidget *print_scale;
  GtkWidget *vbox1;
  GtkWidget *handlebox1;
  GtkWidget *menubar1;
  GtkWidget *hbox3;
  GtkWidget *label4;
  GtkWidget *entry2;
  GtkWidget *hbox1;
  GtkWidget *label1;
  GtkWidget *optionmenu1;
  GtkWidget *menu1;
  GtkWidget *entry1;
  GtkWidget *hbox7;
  GtkWidget *vbox2;
  GtkWidget *hbox8;
  GtkWidget *label6;
  GtkWidget *optionmenu3;
  GtkWidget *menu3;
  GtkObject *spinbutton1_adj;
  GtkWidget *spinbutton1;
  GtkWidget *hbox9;
  GtkWidget *label5;
  GtkWidget *optionmenu2;
  GtkWidget *menu2;
  GtkObject *spinbutton2_adj;
  GtkWidget *spinbutton2;
  GtkWidget *scrolledwindow1;
  GtkWidget *textview1;

  print_scale = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (print_scale), _("\346\242\235\347\242\274\347\247\244\346\223\215\344\275\234\344\273\213\351\235\242"));

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (print_scale), vbox1);

  handlebox1 = gtk_handle_box_new ();
  gtk_widget_show (handlebox1);
  gtk_box_pack_start (GTK_BOX (vbox1), handlebox1, TRUE, TRUE, 0);

  menubar1 = gtk_menu_bar_new ();
  gtk_widget_show (menubar1);
  gtk_container_add (GTK_CONTAINER (handlebox1), menubar1);
  gnome_app_fill_menu (GTK_MENU_SHELL (menubar1), menubar1_uiinfo,
                       NULL, FALSE, 0);

  hbox3 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox3);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox3, TRUE, TRUE, 0);

  label4 = gtk_label_new (_("\347\243\205\347\247\244\351\201\240\347\253\257\345\234\260\345\235\200"));
  gtk_widget_show (label4);
  gtk_box_pack_start (GTK_BOX (hbox3), label4, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label4), GTK_JUSTIFY_LEFT);

  entry2 = gtk_entry_new ();
  gtk_widget_show (entry2);
  gtk_box_pack_start (GTK_BOX (hbox3), entry2, TRUE, TRUE, 0);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

  label1 = gtk_label_new (_("\350\256\200\345\217\226"));
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (hbox1), label1, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);

  optionmenu1 = gtk_option_menu_new ();
  gtk_widget_show (optionmenu1);
  gtk_box_pack_start (GTK_BOX (hbox1), optionmenu1, FALSE, FALSE, 0);

  menu1 = gtk_menu_new ();
  gnome_app_fill_menu (GTK_MENU_SHELL (menu1), menu1_uiinfo,
                       NULL, FALSE, 0);

  gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu1), menu1);

  entry1 = gtk_entry_new ();
  gtk_widget_show (entry1);
  gtk_box_pack_start (GTK_BOX (hbox1), entry1, TRUE, TRUE, 0);
  gtk_entry_set_editable (GTK_ENTRY (entry1), FALSE);

  hbox7 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox7);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox7, TRUE, TRUE, 0);

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox2);
  gtk_box_pack_start (GTK_BOX (hbox7), vbox2, TRUE, TRUE, 0);

  hbox8 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox8);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox8, TRUE, TRUE, 0);

  label6 = gtk_label_new (_("\350\256\200\345\217\226"));
  gtk_widget_show (label6);
  gtk_box_pack_start (GTK_BOX (hbox8), label6, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label6), GTK_JUSTIFY_LEFT);

  optionmenu3 = gtk_option_menu_new ();
  gtk_widget_show (optionmenu3);
  gtk_box_pack_start (GTK_BOX (hbox8), optionmenu3, FALSE, FALSE, 0);

  menu3 = gtk_menu_new ();
  gnome_app_fill_menu (GTK_MENU_SHELL (menu3), menu3_uiinfo,
                       NULL, FALSE, 0);

  gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu3), menu3);

  spinbutton1_adj = gtk_adjustment_new (0, 0, 100, 1, 10, 10);
  spinbutton1 = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton1_adj), 1, 0);
  gtk_widget_show (spinbutton1);
  gtk_box_pack_start (GTK_BOX (hbox8), spinbutton1, TRUE, TRUE, 0);
  gtk_widget_set_usize (spinbutton1, 0, -2);

  hbox9 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox9);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox9, TRUE, TRUE, 0);

  label5 = gtk_label_new (_("\345\257\253\345\205\245"));
  gtk_widget_show (label5);
  gtk_box_pack_start (GTK_BOX (hbox9), label5, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label5), GTK_JUSTIFY_LEFT);

  optionmenu2 = gtk_option_menu_new ();
  gtk_widget_show (optionmenu2);
  gtk_box_pack_start (GTK_BOX (hbox9), optionmenu2, FALSE, FALSE, 0);

  menu2 = gtk_menu_new ();
  gnome_app_fill_menu (GTK_MENU_SHELL (menu2), menu2_uiinfo,
                       NULL, FALSE, 0);

  gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu2), menu2);

  spinbutton2_adj = gtk_adjustment_new (0, 0, 100, 1, 10, 10);
  spinbutton2 = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton2_adj), 1, 0);
  gtk_widget_show (spinbutton2);
  gtk_box_pack_start (GTK_BOX (hbox9), spinbutton2, TRUE, TRUE, 0);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow1);
  gtk_box_pack_start (GTK_BOX (vbox2), scrolledwindow1, TRUE, TRUE, 0);
  gtk_widget_set_usize (scrolledwindow1, -2, 324);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  textview1 = gtk_text_view_new ();
  gtk_widget_show (textview1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), textview1);
  gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview1)),
	_("this\nis\na\ngood\nday"), -1);

  gtk_signal_connect (GTK_OBJECT (print_scale), "delete_event",
                      GTK_SIGNAL_FUNC (on_print_scale_delete_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (print_scale), "show",
                      GTK_SIGNAL_FUNC (on_print_scale_show),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (optionmenu1), "changed",
                      GTK_SIGNAL_FUNC (on_optionmenu1_changed),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (optionmenu3), "changed",
                      GTK_SIGNAL_FUNC (on_optionmenu3_changed),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (optionmenu2), "changed",
                      GTK_SIGNAL_FUNC (on_optionmenu2_changed),
                      NULL);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (print_scale, print_scale, "print_scale");
  GLADE_HOOKUP_OBJECT (print_scale, vbox1, "vbox1");
  GLADE_HOOKUP_OBJECT (print_scale, handlebox1, "handlebox1");
  GLADE_HOOKUP_OBJECT (print_scale, menubar1, "menubar1");
  GLADE_HOOKUP_OBJECT (print_scale, menubar1_uiinfo[0].widget, "menuitem1");
  GLADE_HOOKUP_OBJECT (print_scale, menuitem1_menu_uiinfo[0].widget, "login");
  GLADE_HOOKUP_OBJECT (print_scale, menuitem1_menu_uiinfo[1].widget, "logout");
  GLADE_HOOKUP_OBJECT (print_scale, menuitem1_menu_uiinfo[2].widget, "exit");
  GLADE_HOOKUP_OBJECT (print_scale, menubar1_uiinfo[1].widget, "menuitem4");
  GLADE_HOOKUP_OBJECT (print_scale, menuitem4_menu_uiinfo[0].widget, "about");
  GLADE_HOOKUP_OBJECT (print_scale, hbox3, "hbox3");
  GLADE_HOOKUP_OBJECT (print_scale, label4, "label4");
  GLADE_HOOKUP_OBJECT (print_scale, entry2, "entry2");
  GLADE_HOOKUP_OBJECT (print_scale, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (print_scale, label1, "label1");
  GLADE_HOOKUP_OBJECT (print_scale, optionmenu1, "optionmenu1");
  GLADE_HOOKUP_OBJECT (print_scale, menu1, "menu1");
  GLADE_HOOKUP_OBJECT (print_scale, menu1_uiinfo[0].widget, "weight");
  GLADE_HOOKUP_OBJECT (print_scale, menu1_uiinfo[1].widget, "unit_price");
  GLADE_HOOKUP_OBJECT (print_scale, menu1_uiinfo[2].widget, "serial");
  GLADE_HOOKUP_OBJECT (print_scale, menu1_uiinfo[3].widget, "prog_no");
  GLADE_HOOKUP_OBJECT (print_scale, menu1_uiinfo[4].widget, "control");
  GLADE_HOOKUP_OBJECT (print_scale, menu1_uiinfo[5].widget, "current_unit");
  GLADE_HOOKUP_OBJECT (print_scale, menu1_uiinfo[6].widget, "rd_tare");
  GLADE_HOOKUP_OBJECT (print_scale, menu1_uiinfo[7].widget, "current_user");
  GLADE_HOOKUP_OBJECT (print_scale, menu1_uiinfo[8].widget, "data_time");
  GLADE_HOOKUP_OBJECT (print_scale, menu1_uiinfo[9].widget, "string_length");
  GLADE_HOOKUP_OBJECT (print_scale, entry1, "entry1");
  GLADE_HOOKUP_OBJECT (print_scale, hbox7, "hbox7");
  GLADE_HOOKUP_OBJECT (print_scale, vbox2, "vbox2");
  GLADE_HOOKUP_OBJECT (print_scale, hbox8, "hbox8");
  GLADE_HOOKUP_OBJECT (print_scale, label6, "label6");
  GLADE_HOOKUP_OBJECT (print_scale, optionmenu3, "optionmenu3");
  GLADE_HOOKUP_OBJECT (print_scale, menu3, "menu3");
  GLADE_HOOKUP_OBJECT (print_scale, menu3_uiinfo[0].widget, "rd_format_length");
  GLADE_HOOKUP_OBJECT (print_scale, menu3_uiinfo[1].widget, "rd_plu");
  GLADE_HOOKUP_OBJECT (print_scale, menu3_uiinfo[2].widget, "rd_label");
  GLADE_HOOKUP_OBJECT (print_scale, menu3_uiinfo[3].widget, "rd_label");
  GLADE_HOOKUP_OBJECT (print_scale, spinbutton1, "spinbutton1");
  GLADE_HOOKUP_OBJECT (print_scale, hbox9, "hbox9");
  GLADE_HOOKUP_OBJECT (print_scale, label5, "label5");
  GLADE_HOOKUP_OBJECT (print_scale, optionmenu2, "optionmenu2");
  GLADE_HOOKUP_OBJECT (print_scale, menu2, "menu2");
  GLADE_HOOKUP_OBJECT (print_scale, menu2_uiinfo[0].widget, "wr_tare");
  GLADE_HOOKUP_OBJECT (print_scale, menu2_uiinfo[1].widget, "wr_print_format");
  GLADE_HOOKUP_OBJECT (print_scale, menu2_uiinfo[2].widget, "wr_string");
  GLADE_HOOKUP_OBJECT (print_scale, menu2_uiinfo[3].widget, "strap");
  GLADE_HOOKUP_OBJECT (print_scale, menu2_uiinfo[4].widget, "wr_plu");
  GLADE_HOOKUP_OBJECT (print_scale, spinbutton2, "spinbutton2");
  GLADE_HOOKUP_OBJECT (print_scale, scrolledwindow1, "scrolledwindow1");
  GLADE_HOOKUP_OBJECT (print_scale, textview1, "textview1");

  return print_scale;
}

GtkWidget*
create_about (void)
{
  GtkWidget *about;
  GtkWidget *dialog_vbox1;
  GtkWidget *label2;
  GtkWidget *dialog_action_area1;
  GtkWidget *about_close;
  GtkWidget *alignment1;
  GtkWidget *hbox2;
  GtkWidget *image1;
  GtkWidget *label3;

  about = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (about), _("dialog1"));

  dialog_vbox1 = GTK_DIALOG (about)->vbox;
  gtk_widget_show (dialog_vbox1);

  label2 = gtk_label_new (_("\n\350\213\261\345\261\225\345\257\246\346\245\255\350\202\241\344\273\275\346\234\211\351\231\220\345\205\254\345\217\270\n\347\233\233\351\201\240\345\257\246\346\245\255\350\202\241\344\273\275\346\234\211\351\231\220\345\205\254\345\217\270\n       \n\n       \n         \346\242\235\347\242\274\350\250\210\351\207\215\347\247\244\n\n\n\n\345\217\260\345\214\227\347\270\243\346\226\260\345\272\227\345\257\266\346\251\213\350\267\257235\345\267\267127\350\231\2376\346\250\223\n\n\351\233\273\350\251\261\357\274\232(02)8919-1000\n\345\202\263\347\234\237\357\274\232(02)8919-1177\n\347\266\262\345\235\200\357\274\232http://www.excell-tw.com\n\345\256\242\346\234\215\345\260\210\347\267\232\357\274\2320800-009-969\n\347\265\261\344\270\200\347\267\250\350\231\237\357\274\23212246205\357\274\210\350\213\261\345\261\225\357\274\211\n                02082668\357\274\210\347\233\233\351\201\240\357\274\211"));
  gtk_widget_show (label2);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), label2, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT);

  dialog_action_area1 = GTK_DIALOG (about)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  about_close = gtk_button_new ();
  gtk_widget_show (about_close);
  gtk_dialog_add_action_widget (GTK_DIALOG (about), about_close, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (about_close, GTK_CAN_DEFAULT);

  alignment1 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_widget_show (alignment1);
  gtk_container_add (GTK_CONTAINER (about_close), alignment1);

  hbox2 = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox2);
  gtk_container_add (GTK_CONTAINER (alignment1), hbox2);

  image1 = gtk_image_new_from_stock ("gtk-close", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (image1);
  gtk_box_pack_start (GTK_BOX (hbox2), image1, FALSE, FALSE, 0);

  label3 = gtk_label_new_with_mnemonic (_("\351\227\234\351\226\211"));
  gtk_widget_show (label3);
  gtk_box_pack_start (GTK_BOX (hbox2), label3, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);

  gtk_signal_connect (GTK_OBJECT (about), "delete_event",
                      GTK_SIGNAL_FUNC (on_about_delete_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (about_close), "clicked",
                      GTK_SIGNAL_FUNC (on_about_close_clicked),
                      NULL);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (about, about, "about");
  GLADE_HOOKUP_OBJECT_NO_REF (about, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (about, label2, "label2");
  GLADE_HOOKUP_OBJECT_NO_REF (about, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (about, about_close, "about_close");
  GLADE_HOOKUP_OBJECT (about, alignment1, "alignment1");
  GLADE_HOOKUP_OBJECT (about, hbox2, "hbox2");
  GLADE_HOOKUP_OBJECT (about, image1, "image1");
  GLADE_HOOKUP_OBJECT (about, label3, "label3");

  return about;
}

