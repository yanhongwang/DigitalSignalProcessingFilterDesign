#include <gnome.h>

// 這是用來處理檔案名稱放置的函式
void
store_input_file                       ( GtkFileSelection *file_selection,
                                        gpointer         user_data );


// KIND = 1
void
FIR_Moving_Average                     (GtkButton       *button,
                                        gint            mode      );

// KIND = 2
void
FIR_Window                             (GtkButton       *button,
                                        gint            mode      );

// KIND = 3
void
IIR_Butterworth                        (GtkButton       *button,
                                        gint            mode      );

// KIND = 4
void
IIR_Chebyshev                          (GtkButton       *button,
                                        gint            mode      );

// KIND =5
void
Transfer_function                      (GtkButton       *button,
                                        gint            mode      );

// KIND = 6
void
protodata                              (GtkButton       *button,
                                        gint            mode      );

//void
//protodata                              (GtkButton       *button,
//                                        gboolean        enable_analog,
//					gboolean        enable_decibel);
// 以上函式都是自行宣告的
// 以下函式都是系統宣告的

void
on_swap_clicked                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_exit_clicked                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_save_as_numeral_clicked             (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_rs232_delete_event                  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_spectrum_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_combo_entry_baudrate_changed        (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_combo_entry_port_changed            (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_rs232_show                          (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_save_as_eps_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_move_average_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_window_clicked                      (GtkButton       *button,
                                        gpointer         user_data);

void
on_butterworth_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_chebyshev_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_specification_on_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_coefficient_on_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_fir_input_coefficient_clicked       (GtkButton       *button,
                                        gpointer         user_data);

void
on_iir_input_coefficient_clicked       (GtkButton       *button,
                                        gpointer         user_data);

void
on_print_spectrum_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_clean_data_area_clicked             (GtkButton       *button,
                                        gpointer         user_data);


void
on_inspect_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_button1_clicked                     (GtkButton       *button,
                                        gpointer         user_data);
