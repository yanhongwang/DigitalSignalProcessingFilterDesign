#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <gnome.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

// 是否有登入的動作
static gboolean logined =FALSE;


gboolean
on_print_scale_delete_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	if( logined )
	{
		printf( "logout\n" );
		fflush( stdout );
	
		while( getchar() != '\n');
		while( getchar() != '\n');
	}
	else
	{
		//因為沒有登入過，但是要通知機器作離開的動作
		printf( "192.168.1.193\n");
		fflush( stdout );
	}
	printf( "bye\n" );
	fflush( stdout );
	gtk_exit( 0 );
	return FALSE;
}


void
on_print_scale_show                    (GtkWidget       *widget,
                                        gpointer         user_data)
{
	GtkWidget *logout = lookup_widget( GTK_WIDGET( widget ), "logout" );
	GtkWidget *optionmenu1 = lookup_widget( GTK_WIDGET( widget ), "optionmenu1" );
	GtkWidget *address = lookup_widget( GTK_WIDGET( widget ), "entry2" );
	GtkWidget *optionmenu2 = lookup_widget( GTK_WIDGET( widget ), "optionmenu2" );
	GtkWidget *optionmenu3 = lookup_widget( GTK_WIDGET( widget ), "optionmenu3" );
	
	gtk_widget_set_sensitive( GTK_WIDGET( logout ), FALSE );
//	gtk_widget_set_sensitive( GTK_WIDGET( optionmenu1  ), FALSE );
//	gtk_widget_set_sensitive( GTK_WIDGET( optionmenu2  ), FALSE );
//	gtk_widget_set_sensitive( GTK_WIDGET( optionmenu3  ), FALSE );
	gtk_entry_set_text( GTK_ENTRY( address ) , "192.168.1.193" );
}


void
on_login_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkWidget *login = lookup_widget( GTK_WIDGET( menuitem ), "login" );
	GtkWidget *logout = lookup_widget( GTK_WIDGET( menuitem ), "logout" );
	GtkWidget *optionmenu1 = lookup_widget( GTK_WIDGET( menuitem ), "optionmenu1" );
	GtkWidget *address = lookup_widget( GTK_WIDGET( menuitem ), "entry2" );
	GtkWidget *optionmenu2 = lookup_widget( GTK_WIDGET( menuitem ), "optionmenu2" );
	GtkWidget *optionmenu3 = lookup_widget( GTK_WIDGET( menuitem ), "optionmenu3" );

	gtk_widget_set_sensitive( GTK_WIDGET( login  ), FALSE );
	gtk_widget_set_sensitive( GTK_WIDGET( logout  ), TRUE );
	gtk_widget_set_sensitive( GTK_WIDGET( optionmenu1  ), TRUE );
	gtk_widget_set_sensitive( GTK_WIDGET( optionmenu2  ), TRUE );
	gtk_widget_set_sensitive( GTK_WIDGET( optionmenu3  ), TRUE );
			
	if( !logined )
	{
		logined = TRUE;
		fprintf( stdout,"%s\n",gtk_entry_get_text( GTK_ENTRY( address ) ) );
		fflush( stdout );
		gtk_widget_set_sensitive( GTK_WIDGET( address  ), FALSE );
	}

	printf( "login\n" );
	fflush( stdout );
	while( getchar() != '\n');
	while( getchar() != '\n');
}


void
on_logout_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkWidget *login = lookup_widget( GTK_WIDGET( menuitem ), "login" );
	GtkWidget *logout = lookup_widget( GTK_WIDGET( menuitem ), "logout" );
	GtkWidget *optionmenu1 = lookup_widget( GTK_WIDGET( menuitem ), "optionmenu1" );
	GtkWidget *optionmenu2 = lookup_widget( GTK_WIDGET( menuitem ), "optionmenu2" );      
	GtkWidget *optionmenu3 = lookup_widget( GTK_WIDGET( menuitem ), "optionmenu3" );
		
	gtk_widget_set_sensitive( GTK_WIDGET( login  ), TRUE );
	gtk_widget_set_sensitive( GTK_WIDGET( logout  ), FALSE );
	gtk_widget_set_sensitive( GTK_WIDGET( optionmenu1  ), FALSE );
	gtk_widget_set_sensitive( GTK_WIDGET( optionmenu2  ), FALSE );
	gtk_widget_set_sensitive( GTK_WIDGET( optionmenu3  ), FALSE );

	printf( "logout\n" );
	fflush( stdout );
	while( getchar() != '\n');
	while( getchar() != '\n');
}


void
on_exit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if( logined )
	{
		printf( "logout\n" );
		fflush( stdout );
		while( getchar() != '\n');
		while( getchar() != '\n');
	}
	else
	{
		printf( "192.168.1.193\n");
		fflush( stdout );
	}
	printf( "bye\n" );
	fflush( stdout );
	gtk_main_quit();
			
}

void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	
	GtkWidget *about;
	about = create_about();
	gtk_widget_show( about );
/*
	GtkWidget *textview1 = lookup_widget( GTK_WIDGET( menuitem ), "textview1" );
	GtkTextIter	iter;
	gint i;
	gchar s[100];
	gint len;
	GtkWidget *buffer;
	GtkTextBuffer *buffer;
GtkTextTag *tag;
GtkTextIter start, end;
	buffer  = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview1));

	len = gtk_text_buffer_get_char_count ( buffer);
	fprintf(stderr,"%d\n",len);
	gtk_text_buffer_get_iter_at_offset(buffer, &iter, len);

	for (i=0;i<10;i++) 
	{
		sprintf(s, "%d", i);
		gtk_text_buffer_insert(buffer, &iter, s, -1);
	}
	
	  tag = gtk_text_buffer_create_tag (buffer, "blue_foreground",       "foreground", "yellow", NULL);  
	    gtk_text_buffer_get_iter_at_offset (buffer, &start, 0);
	      gtk_text_buffer_get_iter_at_offset (buffer, &end, 12);
	gtk_text_buffer_delete (buffer, &start, &end);
	      gtk_text_buffer_apply_tag (buffer, tag, &start, &end);
	*/	
}


void
on_optionmenu1_changed                 (GtkOptionMenu   *optionmenu,
                                        gpointer         user_data)
{
	GtkWidget *optionmenu1 = lookup_widget( GTK_WIDGET( optionmenu ), "optionmenu1" );
	GtkWidget *menu = GTK_OPTION_MENU( optionmenu1 ) -> menu;
	GtkWidget *active_item = gtk_menu_get_active( GTK_MENU( menu ) );
	gint mode = g_list_index( GTK_MENU_SHELL( menu ) -> children, active_item );
	GtkWidget *entry1 = lookup_widget( GTK_WIDGET( optionmenu ), "entry1" );

	gint i;
	gchar str[1024];
	gchar c;

	switch( mode )
	{
		case  0 : printf( "read weight\n" ); break;
		case  1 : printf( "read unit_price\n" ); break;
		case  2 : printf( "read serial\n" ); break;
		case  3 : printf( "read prog_no\n" ); break;
		case  4 : printf( "read control\n" ); break;
		case  5 : printf( "read current_unit\n" ); break;
		case  6 : printf( "read tare\n" ); break;
		case  7 : printf( "read current_user\n" ); break;
		case  8 : printf( "read date_time\n" ); break;
		case  9 : printf( "read string_length\n" ); break;
	}
	fflush(stdout);
	while( ( c = getchar() ) != '\n' );
	for(i = 0;(c = getchar()) != '\n';i++)
		str[i] = c;
	str[i] = 0;
	while( ( getchar() ) != '\n' );
	gtk_entry_set_text( GTK_ENTRY( entry1 ), str );
}


void
on_optionmenu2_changed                 (GtkOptionMenu   *optionmenu,
                                        gpointer         user_data)
{

	GtkWidget *spinbutton2 = lookup_widget( GTK_WIDGET( optionmenu ), "spinbutton2" );
	gint channel = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( spinbutton2 ) );
	
	GtkWidget *optionmenu2 = lookup_widget( GTK_WIDGET( optionmenu ), "optionmenu2" );
	GtkWidget *menu = GTK_OPTION_MENU( optionmenu2 ) -> menu;
	GtkWidget *active_item = gtk_menu_get_active( GTK_MENU( menu ) );
	gint mode = g_list_index( GTK_MENU_SHELL( menu ) -> children, active_item );

	GtkWidget *textview1 = lookup_widget( GTK_WIDGET( optionmenu ), "textview1" );
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview1));
	
	gchar str[1024];
	//一定要指定為空，因為一開始的時候連接時是沒有東西
	gchar take_str[1024];
	gint i;
	gchar *str_p;
	GtkTextIter start, end;

	//把在緩衝區的兩邊邊界抓取
	gtk_text_buffer_get_bounds( buffer, &start, &end );
	//轉換utf8的格式為當地並拷貝至指定位置
	strcpy( str , g_locale_from_utf8( gtk_text_buffer_get_text( buffer, &start, &end, TRUE), -1, NULL, NULL, NULL ) );

	while(	str_p =  strchr( str, '\n' ) ) 
	{
		strcpy( take_str,  str_p + 1 );
		i = strcspn( str, "\n");
		str[ i ] = '\\';
		str[ ++i ] = 'n';
		str[ ++i ] = '\0';
		strcat( str, take_str );
	}
//	fprintf(stderr,"%s\n",str);
	switch( mode )
	{
		case 0 : fprintf( stdout, "write tare %s\n", str ); break;
		case 1 : break;
		case 2 : fprintf( stdout, "write string %d %s\n", channel, str ); break;
		case 3 : fprintf( stdout, "write strapp %d %s\n", channel, str ); break;
		case 4 : break;
	}
	fflush( stdout );
	while( ( getchar() ) != '\n' );
	while( ( getchar() ) != '\n' );
}


gboolean
on_about_delete_event                  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	GtkWidget *about  = lookup_widget ( GTK_WIDGET( widget ), "about");
	gtk_widget_destroy(about);
	return FALSE;
}


void
on_about_close_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
        GtkWidget *about  = lookup_widget ( GTK_WIDGET( button ), "about");
	gtk_widget_destroy(about);
}


void
on_optionmenu3_changed                 (GtkOptionMenu   *optionmenu,
                                        gpointer         user_data)
{
	GtkWidget *optionmenu3 = lookup_widget( GTK_WIDGET( optionmenu ), "optionmenu3" );
	GtkWidget *menu = GTK_OPTION_MENU( optionmenu3 ) -> menu;
	GtkWidget *active_item = gtk_menu_get_active( GTK_MENU( menu ) );
	gint mode = g_list_index( GTK_MENU_SHELL( menu ) -> children, active_item );

	GtkWidget *spinbutton1 = lookup_widget( GTK_WIDGET( optionmenu ), "spinbutton1" );
	gint channel = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( spinbutton1 ) );

	GtkWidget *textview1 = lookup_widget( GTK_WIDGET( optionmenu ), "textview1" );
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview1));

	gchar str[1024];
	gchar c;
	gint i;
//	gint fd;
	
	union get_plu plu;

	switch( mode )
	{
		case 0 : fprintf( stdout, "read format_length %d\n", channel ); break;
		case 1 : fprintf( stdout, "read plu %d\n", channel ); break;
		case 2 : fprintf( stdout, "read label %d\n", channel ); break;
		case 3 : fprintf( stdout, "read string %d\n", channel ); break;
	}
	fflush( stdout );

	while( ( c = getchar() ) != '\n' );
//	fd  = open( "/dev/stdin", O_RDONLY | O_NOCTTY );
//	i = read( fd, &plu.seg, sizeof( plu.seg ));
	fread( &plu, sizeof( plu ), 1, stdin);

	
	for( i = 0; i < 136 ; i++ )
		fprintf( stderr, "%c",plu.seg[ i ]);
		
        fflush( stderr);

/*
	for(i = 0;(c = getchar()) != '^' ;i++)
	{	str[i] = c; }
//		fprintf(stderr,"%c",c);
	str[i] = '\0';*/
	// fprintf(stderr,"\n%s\n",str);
	while( ( getchar() ) != '\n' );
//	gtk_text_buffer_set_text (buffer, g_locale_to_utf8( str, -1, NULL, NULL, NULL ), -1);
}


