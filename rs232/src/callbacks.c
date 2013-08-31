#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <glib.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

//#include <iostream.h>

// Ū���B�g�J I/O ���ɭԻݭn�o�Ө禡�A���ɭ԰������ɭԨq�b�ù��W�]��K���I�o�N�s���ҵ��Y�ұo���ASeeing is believing!
#include <stdio.h>

#include <string.h>

// sys/stat.h �ΨӨϥ� stat �o�Ө禡�A�ΥL�ӧP�O�ɮ׬O�_�s�b��J������m
#include <sys/stat.h>

// termios.h�Bfcntl.h ��C�𪺨�w
#include <termios.h>
#include <fcntl.h>

#include <stdlib.h>

// �Ϋܦh�ƾǨ禡�p sin �B cos �B cosh �α`�Ʃw�q M_PI = �k����
#include <math.h>

#define MAX_VALUE 1000
#define LENGTH 100

// �ΦbŪ���ɮ׮ɡA�@�椧�w�]���סA�Ω� fgets �����A�� fgets �u�n�DŪ�즹���סA�άO����Y����
#define SIZE 20

// �Ψө�m�Ȧs�ɮת��A�p .gnuplot�A�� .data ��ӼȮ���
#define TMP_DIR "/tmp/"

// ��C���w���j�v
gint BAUDRATE;

// Ū���@����Ƥ��줸��
gint READSIZE;

// �n�}�Ҫ��ǦC��A�w�]�O ttyS0�A�]�N�O com1
gchar *PORT;

// �N�O�o�i�������� FIR
//                        1 �G ���ʥ����o�i��( Moving average filter )
//                        2 �G �����o�i��( Window filter )�G
//                             �~������ 44 �B�~������ 55 �B�¤H���� 75
//                                                          p.s.�����Ǥ��Ʀr�����a�I��v
//               IIR
//                        3 �G �o���ȱo�o�i��( Butterworth filter )
//                        4 �G ���񳷤��o�i������( Chebyshev filter )
//
//
// �ΰ��]���ಾ��ƥN�J a �Bb �ȡA�Y�� IIR �h�� a �ȡA�Y�� FIR �h�Ȧ� b �ȦӤw�A�b���O��
//                        5 �G ���]���ಾ��Ƥ� IIR �� FIR ���o�i��
gint KIND;

// �i�H���O�ѪR�סA�]�N�O�ȶV�j�A�ϧΪ��u���V���ơA���O�@��ӻ��A��� 100 �N�w�g�۷���ΤF�A�A�j�]�u�O���O�ɶ��Ӥw
gint pixel;

// �}�� tmp_data_name �ɩҥΪ��ɮ׫��СA�]�������ܼƬO�]���C���o�i�����禡���ݭn�o���ɮ׫���
FILE *tmp_data;

// �W�йϤ����`�ȡA�Y�O���ƫר������@�ˡA�h�ȷ|���ܳ�I
gfloat total = 0;

// �W�йϤ����̤j��
gfloat max = 0;

// �ŧi�������ܼơA���j�a�O�����֦P�ɡ����A�۹諸�A�p�G�ڪ��{�������D���ܡA�|�ܦ��������P���F�ڡI
GtkWidget *statusbar;

// �s�@�@�ӥ��쪺 widget ���СA�n���Ҧ����禡�i�H��� widget ���СA
// �b�o�̬O���S�O���Φb file_selection_dialog
GtkButton *some_button;

// �ŧi�@�ӥ��쪺 widget �O���ɦW�� file_selection_box �A�Ӧb
// on_save_as_numeral_clicked �ƥ�QĲ�o���ɭԥi�H���ͤ@�ӹϧάɭ��A�n�� store_input_file �i�H
// ��� selection dialog
GtkWidget *file_selection_box;



void test( gint num );



// on_rs232_show �O�N�� rs232 �o�ӵ{���b�@�}�l���ɭԴN���檺�禡�A�]�N�O�u������
// �o�@���A�]�N�O���A��{���bŪ�� main.c ���A����� gtk_widget_show (rs232)
// �o�Ө禡�A�ߨ���� on_rs232_show �o�Ө禡�C���]���u����o�@���A�ҥH�ڭ̥i�H��
// ��Өt�Ϊ���]�Ȳβγ]�w�b���B�C
void
on_rs232_show                          (GtkWidget       *widget,
                                        gpointer         user_data)
{
   // �n�ΰ��]���ಾ��ƫY�ƿ�J������
   GtkWidget *fir_input_coefficient = lookup_widget( GTK_WIDGET( widget ), "fir_input_coefficient" );
   GtkWidget *iir_input_coefficient = lookup_widget( GTK_WIDGET( widget ), "iir_input_coefficient" );
   GtkWidget *a_value = lookup_widget( GTK_WIDGET( widget ), "a_value" );
   GtkWidget *b_value = lookup_widget( GTK_WIDGET( widget ), "b_value" );

   // �@�}�l���]�٤��ݭn�C�L����F��
   GtkWidget *printer_name = lookup_widget( GTK_WIDGET( widget ), "printer_name" );

   // �Ʊ�����@���}���ɭԹw�]�O�H��J���ӳW�檺�o�i��
   // ���]���ಾ��ƫY�ƿ�J������A��������
   gtk_widget_set_sensitive( GTK_WIDGET( fir_input_coefficient ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( iir_input_coefficient ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( a_value ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( b_value ), FALSE );

   gtk_widget_set_sensitive( GTK_WIDGET( printer_name ), FALSE );

   statusbar = lookup_widget( GTK_WIDGET( widget ), "statusbar1");

   // �w�]�� 4800 ���j�v
   BAUDRATE = B4800;

   // Ū���@����ƨ�줸��
   READSIZE  = 16;

   // �n�}�Ҫ��ǦC��A�w�]�O ttyS0�A�]�N�O com1
   PORT = "/dev/ttyS0";

   // �w�]���o�i�������A���ҿץη�M�O�γ̦n���o�A�b���O���񳷤��o�i��
   //KIND = 4;
   KIND = 6;
}

void
on_swap_clicked                      (GtkButton       *button,
                                        gpointer         user_data)
{
   // �s���l��Ʀp�GUS,NT,+  14.14 �o�˪�����
   gchar buf[ READSIZE + 1 ];

   // extract ���O�f���}���~�P( �L�}�f���} )�A�ӬO�� buf �ұo�쪺�¼ƭȸ��
   // �p�G buf = US,NT,+  14.14 �o�˪��ܡA�h extract = +  14.14
   gchar *extract;

   // �P�O�O�_ toggle_button �O�_�H�U�h�A�Y�O�h�� true
   gboolean enforce_write;

   // �N�O�@�ӭn�Q�g�J����ɪ��ɦW����
   FILE *savefile = NULL;

   // oldtio �O�ǳƥΨө�m��Ӫ���C�𪺳]�w�Anewtio �O�Ψө�m�n�Ψӥ��}��C�𪺳]�w
   struct termios oldtio;
   struct termios newtio;

   // �ΨӴ��եثe�ҧƱ檺���|���ɦW�A�O�_���w�s�b�A
   struct stat examine;

   // �Ψө�m���V�ǦC�𪺫���
   gint fd;

   // �Ψө�n�J�s��ƪ����|�ɦW
   gchar *path;

   gint i = 0, num;

   // �ѯ��L�ҶǥX���g�B�z����l��ƭȡA��m�b text ���q�X��
   GtkWidget *protodata = lookup_widget(GTK_WIDGET(button), "text1");

   // text �b����ɭԬO��m�w�B�z�L���ƭȸ��
   GtkWidget *numeral = lookup_widget(GTK_WIDGET(button), "text2");

   // �o�� button �� checkbutton ���@�˪��Ϊk�A������@�˪��O�A�L�i�H���r�����L�b
   // ���s�A���]���p���A�ҥH�L�� button �]����j�A�O�i�H���ܤj�p���A �o���ܼƦb�����N��
   // �O������U����A�h��}�Ҽg�J�ɮ׮ɡA���ި����ɮץ��e�O�_�s�b�A���@�w�j���g�J�A
   GtkWidget *canwrite = lookup_widget( GTK_WIDGET( button ), "togglebutton1");

   // ��M�Ψө�m�ǳƦ��X����ƭnŪ���� entry
   GtkWidget *amount = lookup_widget( GTK_WIDGET( button ), "entry2");

   // ��M�Ψө�m��w���Ʀ줸�ժ��ת� entry
   GtkWidget *bytes = lookup_widget( GTK_WIDGET( button ), "entry5" );

   // ��M�n�g�J������ɸ��|�Ψ��ɮת� entry
   GtkWidget *saveaspath = lookup_widget( GTK_WIDGET( button ), "entry1" );

   // �]���b������ƪ��ɭԡA�|�ܤ[�A�ҥH�S�a���@�Ӷi�״ΡA�Ӫ�ܥثe������
//   GtkWidget *progressbar1 = lookup_widget( GTK_WIDGET( button ), "progressbar1" );

   // path ��ۭn�g�J������ɸ��|�Ψ��ɮ�
   path = gtk_entry_get_text( GTK_ENTRY( saveaspath ) );

   // �]�� gtk_entry_get_text �ұo�쪺�^�ǭȬO�@�Ӧr��ȡA�ҥH�� atoi �ഫ���Ʀr
   num = atoi( gtk_entry_get_text( GTK_ENTRY( amount ) ) );

   // �]�� gtk_entry_get_text �ұo�쪺�^�ǭȬO�@�Ӧr��ȡA�ҥH�� atoi �ഫ���Ʀr
   READSIZE = atoi( gtk_entry_get_text( GTK_ENTRY( bytes ) ) );

   // ��O�ثe���� toggle_button �O�H�U�h���A�٬O�B�_�ӡA�@���O�_�n�j���g�J�ɮת��̾�
   enforce_write = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( canwrite ) );


   // �� enforce_write �� true �ɡA�άO���w���ɮפ��s�b�ɫh�i�g�J
   // stat �����~�T���i�H�Ѧ� /usr/include/asm/errno.h �o���ɮ�
   if( ( stat( path, &examine ) == ( 0 ) ) && ( !enforce_write ) )
      // ���ɮצ��w�s�b�t�Τ����ɡA�ҵ������A�C����
      gtk_statusbar_push( GTK_STATUSBAR( statusbar ), 1
      , "���w���ɮצ��w�s�b�ɮרt�Τ��A�п����L���ɮסA�άO���U���j��g�J�ɮס��A�j���g�J�ɮסC" );

      // ��}�Ҹ��|�ɮ� path ���Ѫ��ɭԡA�b���A�C�W��ܥXĵ�i�H���A
      // �a�ϥ��Ѥ]�~�����A�ҥH�S���[�J return�A�]�N�O���A�Y�Ϥ��s�J�ɮפ]�i�H�ǤJ���
   else if( ( savefile = fopen( path , "w" ) ) == NULL )
      gtk_statusbar_push( GTK_STATUSBAR( statusbar ), 1
      , "�A���w���ɮצW�ٵL�k�إߡA�ЦA�T�{�@���C" );


   // PORT �����}�Ҫ��ǦC�˸m��w�]�O com1 �]�N�O /dev/ttyS0�AO_NOCTTY
   // �o�ӰѼƬO�Ʊ�o�Ӧ�{�����n���Q��@�O����׺ݪ��Ҧ�( controlling terminal )
   // �A���y�ܻ��A�]�N�O�ä��|�]���A���F Ctrl + C �M��N�������{�����i��A
   // �� O_RDWR �o�ӰѼƫh�O���˸m�i�Q��Ū���S�i�Q���g���J
   fd = open( PORT, O_RDWR | O_NOCTTY );

   // ��}�Ҹ˸m���ѮɡAfd < 0 �A�M��b���A�C�W���q�X�T��ĵ�i
   // �� fd > 0 ���ɭԡA�h�}�l�i�J��]�ǦC�𪺶��q
   if ( fd < 0 )
   {
      gtk_statusbar_push( GTK_STATUSBAR( statusbar ), 1
      , "���w���˸m�}�ҥ��ѡA�Цۦ�d����]�C" );
      if ( savefile != NULL )
         fclose( savefile );
      return;
   }
   else
   {  // �_�Ǫ��O�A�b�q�b���T���C�����r���Y�O�[�J���\���o�Ӧr�A compile �N�|�X���D
      gtk_statusbar_push( GTK_STATUSBAR( statusbar ), 1
      , "���w���˸m�w�}�ҡC" );

      // ����e����?C�𪺳]�w�J�s�_��
      tcgetattr( fd, &oldtio );

      // bzero �|�N newtio �ҫ����O����ϰ�e sizeof( newtio ) �Ӧ줸�աA
      // �����]���s�ȡA�۷��I�s memset( void *s ), 0,size_t n );
      bzero( &newtio, sizeof( newtio ) );

      // �b /usr/include/bits/termios.h ������ termios �����c��A�p�G
      // c_cflag�Bc_iflag�Bc_oflag�Bc_lflag�Bc_cc ���X�г]�w�ȥi�b�����

      // c_cflag �o�ӺX�б�����j�v�B�줸�ơB�P�줸�B����줸�H�εw��y�q����
      // ��̫ᵲ�G�ѡ�|�� bitwise operator �v��B�⤸�ұo��?����G�ӨM�w
      // BAUDRATE �]�w�ǿ骺�t�v�A
      // CRTSCTS ��X�y�q���w��y����
      // CS8 �� 8n1�A8 �줸�ˬd�A�@�Ӳפ�줸( character stop 8 )
      // CLOCAL ���a�s�u�A����ƾ�?�����\��
      // CREAD �ϭP�౵���r��
      newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;

      // c_iflag �o�ӺX�ШM�w�p��B�z�����쪺�r���A�Y���[�B�z�h�O�� 0�Y?? raw�Ҧ�
      // ��̫ᵲ�G�ѡ�|��bitwise operator �v��B�⤸�ұo�쪺���G�ӨM�w
      // IGNPAR �g�P�줸�ˬd��A���n�޿��~���o��
      // ICRNL �N CR ������ NL �A�_�h���J�T���� CR �ɤ��|�פ��J?A
      // �A���M�p�G�S����L����J�B�z�����ΤU�A�]�i�H��˸m�]�w�� raw �Ҧ��A
      newtio.c_iflag = IGNPAR | ICRNL;

      // c_iflag �M�w�F��X�Ҧ��A�M c_iflag �@�ˡAraw ��l���Ҧ���X�h�O�� 0
      newtio.c_oflag = 0;

      // c_lflag �o�ӺX�ЬO�@�ӨM�w�p��B�z�ѧǦC�˸m�ұo�쪺�r���A�@��ӻ��q�`
      // ���]�w�� canonical �άO raw ���Ҧ��A?]�N�O���L ICANON �άO 0 �䤤�@�ӭ�
      // ICANON ���P��зǿ�J�A�ϩҦ��^������ΡA�ä��e�X�H���H�K��I�s�{��
      newtio.c_lflag = 0;

      // c_cc �o�Ӱ}�C�]�t�F�@�ǹ��O timeout ���Ѽƪ��r�q

      // VINTR �ҥN���}�ȴN�O���_( interrupt )�A�N�O Ctrl + C
      newtio.c_cc[ VINTR ] = 0;

      // VQUIT �ҥN���}�ȴN�O���( quit )�A�N�O Ctrl + Z
      newtio.c_cc[ VQUIT ] = 0;

      // VERASE �ҥN���}�ȴN�O�M��( erase )�A�N�O backspace
      newtio.c_cc[ VERASE ] = 0;

      // VKILL �ҥN���}�ȴN�O�M���@��( kill-line )�A�]�N�O Ctrl + U
      newtio.c_cc[ VKILL ] = 0;

      // VEOF �ҥN���}�ȴN�O��̫�@��( end of file )�A�]�N�O Ctrl + D
      newtio.c_cc[ VEOF ] = 4;

      // ���ϥΤ��Φr���ժ��p�ɾ�
      newtio.c_cc[ VTIME ] = 0;

      // VMIN �ҥN���}�Ȫ�ܬO�nŪ�����̤p�줸�ƥ�
      newtio.c_cc[ VMIN ] = READSIZE;

      // '\0'
      newtio.c_cc[ VSWTC ] = 0;

      // Ctrl + Q
      newtio.c_cc[ VSTART ] = 0;

      // Ctrl + S
      newtio.c_cc[ VSTOP ] = 0;

      // '\0'�A�]�N�O Carriage return ( CR )�A�]�N�O Enter
      newtio.c_cc[ VEOL ] = 0;

      // Ctrl + R
      newtio.c_cc[ VREPRINT ] = 0;

      // Ctrl + u
      newtio.c_cc[ VDISCARD ] = 0;

      // Ctrl + w
      newtio.c_cc[ VWERASE ] = 0;

      // second end-of-line �N�O LF
      newtio.c_cc[ VEOL2 ] = 0;

      // �M���]�w
      tcflush( fd, TCIFLUSH );

      // �N newtio ���]�w�A�B TCSANOW �R�O��]�w�ߧY�ͮ�
      tcsetattr( fd, TCSANOW, &newtio );

      // �e�@����ơA�����B�z�A�קK�]����Ƥ����㪺����
      bzero( buf, sizeof( buf ) );
      read( fd, buf, READSIZE );

      while( ( i++ ) != num )
      {
         // �� buf �o���O����M�����b
         bzero( buf, sizeof( buf ) );

	 // Ū���� fd ���V���ɮ׸˸m����ơA���]�N�O�ɮ׫��ЬO����/dev/ttyS
	 // �A�M��N��줸�ƥج� READSIZE ���줸�աA�e��ҫ����O����A�]�N�O buf
	 // read ���Ǧ^�Ȭ����Ū���쪺�줸�ռơA�p�G�Ǧ^ 0 �A��ܤw��F�ɮ׵���
	 // �Ϊ̬O�L�iŪ������ơA���~�ɮ�Ū�g��m�|�HŪ���쪺�줸�ղ��ʡC
	 // �P�_�O�_�ҶǦ^���줸�լO�_�j�p�@�P�A�T�w�S��Ū���A�u�n�����A�����j��N���n���C
	 if( read( fd, buf, READSIZE ) != READSIZE )
            continue;

	 // strpbrk �o�Ө禡�O�Ψӧ�X�޼� buf �r�ꤤ�A�̥��X�{�s�b�޼ơ�+-�����r
         // �]����l�����B�z��Ƭ� US,NT,+  14.14 �o�˪������A�b��
	 // buf = US,NT,+  14.14 �A�Ӹg�L�禡�B�z��Aextract = +  14.14 �o�˪���
//	 extract = strpbrk( buf, "+-");

	 // �b���Y buf = US,NT,+  14.14 �A�h extract = 14.14�A�]�N�O���a���t��
	 extract = strpbrk( buf, "0123456789" );

         // �� buf ����ƴ��J protodata ���A�� extract ����ƴ��J
         // numeral ���� gtk_text_insert �o�Ө禡�A�Ĥ@�ӰѼƬ����w�� text �A
	 // �Ĥ@�� NULL ���ѼƬ��ϥΪ��r���ANULL ��ܥΥثe���r��
	 // �ĤG�� NULL ���ѼƬ��o�� text �̪��r���C��A NULL ���Υثe���C��
         // �ĤT�� NULL ���ѼƬ��o�� text �̪��I���C��A NULL ���Υثe���C��
	 // �ĥ|�� buf ���ѼƬ��n���J���r��A
	 // �Ĥ��ӰѼƦ�m�h�� buf ���j�p�A�Y�� -1 �ȡA�h��ܥ����q�X��
	 gtk_text_insert( GTK_TEXT( protodata ), NULL, NULL, NULL, buf, -1 );
         gtk_text_insert( GTK_TEXT( numeral ), NULL, NULL, NULL, extract, strlen( extract ) );

     //    gtk_timeout_add( 1., test, num );


         // �T�w���e�ɮ׬O�_���}�Ҧ��\�A�p�G���h�g�J�ɮפ���
	 if ( savefile != NULL )
	 {
	    fprintf( savefile, "%s", extract );
	    fflush( savefile );
	 }

      }

      // ��j�鵲���ɡA�]�N�O num ����Ʊ������ɭԡA���ɡA�� oldtio ���]�w
      // ���ߧY���g�^ fd �ҫ����˸m�ɮפ�
      tcsetattr( fd, TCSANOW, &oldtio );

      // �T�w���e�ɮ׬O�_���}�Ҧ��\�A�p�G���h�����ɮ�
      if( savefile != NULL )
         fclose( savefile );
   }
}

void test( gint num )
{

 //GtkWidget *appbar1 = lookup_widget( GTK_WIDGET( some_button ), "appbar1" );
 GtkWidget *progressbar1 = lookup_widget( GTK_WIDGET( some_button ), "progressbar1" );
 static gint i = 1;

// gfloat j = 500, percent;
//g_print("it's worked\n");
      i++;
  // percent = percent * 100;

  // if( i != 5000 )
  //    gnome_appbar_set_progress( GNOME_APPBAR( appbar1 ), percent );
   //else
   {
   //   gnome_appbar_set_progress( GNOME_APPBAR( appbar1 ), 0 );
      gtk_progress_set_value( GTK_PROGRESS( progressbar1 ), ( i / num ) * 100 );
   // i = 0;
    //getchar();
    }
}

void
on_combo_entry_baudrate_changed        (GtkEditable     *editable,
                                        gpointer         user_data)
{
   GtkWidget *baudrate = lookup_widget( GTK_WIDGET( editable ), "combo_entry_baudrate");

   // /usr/include/bits/termios.h ���ۨ�w�q��
   // �@���ϥΪ̦��۷Q�n�����j�v���N�ϡA�h changed ���ƥ�|�ߧYĲ�o�A BAUDRATE �K�|����
   switch( atoi( gtk_entry_get_text( GTK_ENTRY( baudrate ) ) ) )
   {
      case  2400 : BAUDRATE =   B2400; break;
      case  4800 : BAUDRATE =   B4800; break;
      case  9600 : BAUDRATE =   B9600; break;
      case 19200 : BAUDRATE =  B19200; break;
      case 38400 : BAUDRATE =  B38400; break;
      case 57600 : BAUDRATE =  B57600; break;
      case 115200: BAUDRATE = B115200; break;
   }

   // �U���o�Өq�X�ӥi�H�ݨ� BAUDRATE �b /usr/include/bits/termios.h ���w�q��
   // ���O�ѹ껡�A��ı�o�ǩǪ��A���M�{���O���楿�`���C
   // g_print(" %d\n ", BAUDRATE );
}



void
on_combo_entry_port_changed            (GtkEditable     *editable,
                                        gpointer         user_data)
{
   GtkWidget *port = lookup_widget( GTK_WIDGET( editable ), "combo_entry_port" );

   // �@���ϥΪ̦��۷Q�n���ܧǦC�𪺷N�ϡA�h changed ���ƥ�|�ߧYĲ�o�A PORT �K�|����
   switch( atoi( gtk_entry_get_text( GTK_ENTRY( port ) ) ) )
   {
      case 1 : PORT = "/dev/ttyS0"; break;
      case 2 : PORT = "/dev/ttyS1"; break;
   }
}

void
on_exit_clicked                        (GtkButton       *button,
                                        gpointer         user_data)
{
   gtk_main_quit();
}


// ��q file_selection_box �M�w�᪺�ɦW��� entry1 ����
// �A�p�G�������ܡA��M�O���n��J�o�I
void store_input_file( GtkFileSelection *file_selection, gpointer user_data)
{
   // �q file_selection_box �o�Ӫ� path �A��J�󦹳B
   gchar *in_filename;

   GtkWidget *file_path;

   // �P�_�O�n�M�w eps �����|�A�٬O numeral ��
   if( user_data == "eps" )
      // ��ܬO eps ���|���n�D
      file_path = lookup_widget( GTK_WIDGET( some_button ), "entry4" );
   else
      // ��ܬO numeral ���|���n�D
      file_path = lookup_widget( GTK_WIDGET( some_button ), "entry1" );

   // �q file_selection_box �����o���ɦW�Ψ���|�C
   in_filename = gtk_file_selection_get_filename
      ( GTK_FILE_SELECTION( file_selection_box ) );

   // ���ɦW�Ψ���|�W��� entry1 �̭��A�]�N�O file_path �աI
   gtk_entry_set_text( GTK_ENTRY( file_path ), in_filename );
}

void
on_save_as_numeral_clicked                      (GtkButton       *button,
                                        gpointer         user_data)
{
   // ���w button �����Ы��� some_button �A�� store_input_file
   // �o�Ө禡�i?H���ݭn��widget
   some_button = button;

   // �гy�@�� file_selection_box�A�O����D�� Choose the data file.....
   file_selection_box = gtk_file_selection_new( "�Цۦ�[�W���ɦW�A�p�����(txt)�ι���(eps�Bpdf�Bps�K�K)" );

   // �]�����D�Ӫ��F�A��?H�ڧ�o���ɮ׿�ܳo�ӵ������ܤF�@�U�j�p
   gtk_widget_set_usize (file_selection_box, 700, -2);

   // �� file_selection_box�̭��� "ok" ���s�@�ƥ�Ĳ�o�� store_input_file ���禡
   // �U��ݭ��i�_�`�N�� gtk_signal_connect �M gtk_signal_connect_object �����P�ܡH
   // ��h�W�o��Ө禡�O�@�˪��A�ߤ@���P���a��O gtk_signal_connect_object �i�H��
   // widget �A�]�N�O���ƹ�W�A gtk_widget_destroy �o�Өƥ�A�O�ݭn�@�� widge ���Ѽ�
   // ���M�L��򪾹D�n�R������
   gtk_signal_connect( GTK_OBJECT
      ( GTK_FILE_SELECTION( file_selection_box ) -> ok_button ),
      "clicked", GTK_SIGNAL_FUNC( store_input_file ), user_data );

   // �b?��U��ok���o�ӫ��s����� file_selection_box �o�� widget
   // �����A�N�O�����b�ù��W�աI
   gtk_signal_connect_object( GTK_OBJECT
      ( GTK_FILE_SELECTION( file_selection_box ) -> ok_button ),
      "clicked", GTK_SIGNAL_FUNC( gtk_widget_destroy ),
      ( gpointer )file_selection_box );

   // �b���U��cancel���o�ӫ��s����� file_selection_box
   // �o�� widget �����A�N�O�����b�ù��W�աI�ӨS���@ store_input_file ���ʧ@
   gtk_signal_connect_object( GTK_OBJECT
      ( GTK_FILE_SELECTION( file_selection_box ) -> cancel_button ),
      "clicked", GTK_SIGNAL_FUNC( gtk_widget_destroy ),
      ( gpointer )file_selection_box );

   // �q�X file_selection_box �o�ӵ����X�ӡI
   gtk_widget_show( file_selection_box );
}

// on_rs232_delete_event �o�Өƥ�|�o�ͦb��A�I��F�����k�W�����������A
// �~�|�o�͡A�]�N�O�ҿ�?? window manager
gboolean
on_rs232_delete_event                  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{

   // gtk_main_quit() �o�Ө禡�O���D���������R�����\��A�����D��L���p widget
   // ���ݭn�h�R���ܡH�۫H�A���o�˪��ðݡH�Ь� main.c ���� gtk_widget_show (rs232);
   // �b GTK+ �{���]�p���[��?��A�e�� container ���[���O�Q�����n���A�����
   // GtkWindow �N�O�@�ӡ��̡��j���e���A��L�٦��Ѧp Horizontal box �٬O
   // Vertical box �H�� Table �]�O�e�����@�ءA���L�L�̳��O�]�t�b GtkWindow �o
   // �ӳ̤j���e���̭��C�ҥH�@���ΤF gtk_main_quit() �N�����Ҧ��]�t�b GtkWindow
   // ���U������ββפ�I
   //gtk_main_quit();

   // �פ�Ҧ��{���A�åB�Ǧ^ exit code ���I�s�̡A�åB����Ҧ����۩� GTK ���귽

   gtk_exit( 0 );


   // �z�פW�U���o�@�椣�|�����A���O�O���F�sĶ����q�L
   // �@���Ǧ^�Ȭ� TRUE �h�t�δN�|���d�������ʧ@�A�� FALSE �O�Ϩt�εo�X destroy ���T��
   return FALSE;
}

void
on_spectrum_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
   // ��� optionmenu1 ���ثe�ȡA�H�K�M�w X �B Y �b�Φ�ؤ覡��{
   GtkWidget *optionmenu1 = lookup_widget( GTK_WIDGET( button ), "optionmenu1" );
   GtkWidget *menu = GTK_OPTION_MENU( optionmenu1 ) -> menu;
   GtkWidget *active_item = gtk_menu_get_active( GTK_MENU( menu ) );
   gint mode = g_list_index( GTK_MENU_SHELL( menu ) -> children, active_item );


   // ���ȨM�w�n���n�N�W�йϥΦL���ø�X
   GtkWidget *print_spectrum = lookup_widget( GTK_WIDGET( button ), "print_spectrum" );

   // ���ȨM�w�n���n�Τj�i���W�й�
   GtkWidget *big_spectrum = lookup_widget( GTK_WIDGET( button ), "big_spectrum" );

   // �� widget �ӧ�����W�����W�v���ȡA�M��q�b���ɪ��k�W���A�]���o�ӭȳ̭��n�A�ҥH�q�b��
   GtkWidget *fs_value = lookup_widget( GTK_WIDGET( button ), "fs" );

   //
   GtkWidget *fir_input_coefficient = lookup_widget( GTK_WIDGET( button ), "fir_input_coefficient" );

   // �N�O eps ����X���|
   GtkWidget *saveaspath = lookup_widget( GTK_WIDGET( button ), "entry4" );

   // �N�O X �b���ѪR��
   GtkWidget *point = lookup_widget( GTK_WIDGET( button ), "point" );

   // �L������q���W��
   GtkWidget *printer_name = lookup_widget( GTK_WIDGET( button ), "printer_name" );

   // �M�w�O�_�ϥΦL����N�W�ЦL�X
   gboolean enable_printer;

   // --------------------------------------------------------------
   // tmp_script_name �O�Ψө� gnuplot �nŪ���� script �R�O
   // tmp_data_name �O�Ψө� data �]�N�O�W�йϪ� X�BY ���ɮת��ɮ�
   // tmp_data_name �N�n�Q gnuplot Ū�J������ɮסA�]�N�O�̭�����ƱN�����A
   // �Ĥ@��O�b�W�ФW X ���ȡA�ĤG��O�b�W�ФW Y ���ȡA��ӭȦ��@���I�A�M�|�����ϧ�
   // ��X�b output_file_name ����
   gchar tmp_script_name[ LENGTH ], tmp_data_name[ LENGTH ];

   // �}�� tmp_script_name �ɩҥΪ��ɮ׫���;
   FILE *tmp_script;

   gchar sys_command[ LENGTH ];

   // �n��X�� eps ���ɮצW��
   gchar *output_file_name;

   // �b eps �ɤ����ҥΨ쪺�ѼơA���D�A X �y�ХH�� Y �y�Ъ��W�١Axlabel �ٻݭn��L�@�ǰT��
   // title �b switch ���ﶵ���p�G�S�������@�Ӫ�]�ȡA�h�|��ĵ�i�A
   gchar xlabel[ LENGTH ] = "", *ylabel = "", temp[ LENGTH ] = "", *title = "";

   // �����W�Ы�A�b�T���C����ܪ���T
   gchar *spectrum_message = "";

   // ��b�W�Ъ��k�W����
   gchar banner[ LENGTH ] = "fs = ";


 /*
   // b �ȴN�O�ಾ��ƪ��Y�ƭȡA�]���O FIR �o�i���A�ҥH�u�� b �ȡA�S�� a ��
   gfloat *b = 0;

   gint i = 0;
*/
   // -------------------------------------------------------------

   // �M�w�O�_�ϥΦL����L�X���G
   enable_printer = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( print_spectrum ) );

   // �M�w X �b���ѪR�צp��
   pixel = atoi( gtk_entry_get_text( GTK_ENTRY( point ) ) );

   system("clear");

   // �� getpid() �o�Ө禡�H�����ͤ@�Ӧb /tmp/ ���U�B���ɦW�� gnuplot ��script�ɡA
   // ��ɭԥi�� gnuplot �hŪ���C
   sprintf( tmp_script_name, "%s%d", TMP_DIR, getpid( ) );
   strcat( tmp_script_name, ".gnuplot");

   // �� getpid() �o�Ө禡�H�����ͤ@�Ӧb /tmp ���U�B���ɦW�� data ������ɡA
   // �i�ѵ� gnuplot ��ƨåB�ӹ��X�ϧ�
   sprintf( tmp_data_name, "%s%d", TMP_DIR, getpid( ) );
   strcat( tmp_data_name, ".data");

   // ���} script �M data �A�u�n�䤤�@�ӥ����}�N���C
   if( ( ( tmp_script = fopen( tmp_script_name, "w" )) == NULL ) ||
      ( ( tmp_data   = fopen( tmp_data_name,   "w" )) == NULL ) )
   {
      gtk_statusbar_push( GTK_STATUSBAR( statusbar ), 1
	 , "�b���ͼȮɪ� script �٬O data �ɮת��ɭԡA�o�͵L�k�}�Ҫ����ΡC" );
      return;
   }

   // �]�w�n��X�� eps �ɮסA�Ψ��m
   output_file_name = gtk_entry_get_text( GTK_ENTRY( saveaspath ) );

   // ��ܬO�έ�?@���o�i��
   switch( KIND )
   {
      case 1 :
      {
         FIR_Moving_Average( button, mode );
	 spectrum_message =
	 "���ʥ����o�i�����W�й��ɤw�g���n�F�A�䵲�G�N��b�䡧�x�s���|����";
	 break;
      }
      case 2 :
      {
         FIR_Window( button, mode );
	 spectrum_message =
	 "�����o�i�����W�й��ɤw�g���n�F�A�䵲�G�N��b�䡧�x�s���|����";
	 break;
      }
      case 3 :
      {
         IIR_Butterworth( button, mode );
         spectrum_message =
	 "���o�ȱo�o�i�����W�й��ɤw�g���n�F�A�䵲�G�N��b�䡧�x�s���|����";
	 break;
      }
      case 4 :
      {
         IIR_Chebyshev( button, mode );
	 spectrum_message =
	 "���񳷤��o�i�����W�й��ɤw�g���n�F�A�䵲�G�N��b�䡧�x�s���|����";
	 break;
      }
      case 5 :
      {
         Transfer_function( button, mode );
	 spectrum_message =
	 "�Ѱ��]���Y�Ʃҧ������W�й��ɤw�g���n�F�A�䵲�G�N��b�䡧�x�s���|����";
	 break;
      }
      case 6:
      {
         protodata( button, mode );
	 spectrum_message =
	 "�Ѱ��]���Y�Ʃҧ������W�й��ɤw�g���n�F�A�䵲�G�N��b�䡧�x�s���|����";
	 break;
       }
   }


   // �� max ���ȥ[�i�h
   strcat( xlabel,"max = " );
   sprintf( temp, "%f,", max );
   strcat( xlabel, temp );
   max = 0;

   // �� total ���ȥ[�i�h
   strcat( xlabel," total = " );
   sprintf( temp, "%f", total );
   strcat( xlabel, temp );
   total = 0;

   // X Y �b���i��զX�|���|�ر��p
   switch( mode )
   {
      case 0 :
      case 1 :
      {
         // �H�Ʀ��W�v���� X �b��ܤ覡�A�åB����a�찾�k��h�A�~���|�� total ���Ȧ��ҲV��
         strcat( xlabel,"           omega( PI )" );

	 // �Ʀ��W�v�U���@��W�q
         ylabel = "| H( omega ) |";

	 break;
      }
      case 2 :
      {
         // �H�Ʀ��W�v���� X �b��ܤ覡�A�åB����a�찾�k��h�A�~���|�� total ���Ȧ��ҲV��
         strcat( xlabel,"           omega( PI )" );

	 // �����W�v�U�����q�W�q
         ylabel = "| H( f ) |( decibel )";

	 break;
      }
      case 3 :
      {

         // �H�����W�v���� X �b��ܤ覡�A�åB����a�찾�k��h�A�~���|�� total ���Ȧ��ҲV��
         strcat( xlabel,"                    f( HZ )" );

	 // �Ʀ��W�v�U���@��W�q
         ylabel = "| H( omega ) |";
	 break;
      }
      case 4 :
      {
         // �H�����W�v���� X �b��ܤ覡�A�åB����a�찾�k��h�A�~���|�� total ���Ȧ��ҲV��
         strcat( xlabel,"                    f( HZ )" );

	 // �����W�v�U�����q�W�q
         ylabel = "| H( f ) |( decibel )";
	 break;
      }
   }

   switch( KIND )
   {
      // �]���U���o�i�����ͤ��P�����D
      case 1 : title = "FIR Moving Average filter"; break;
      case 2 : title = "FIR Window filter";         break;
      case 3 : title = "IIR Butterworth filter";    break;
      case 4 : title = "IIR Chebyshev filter";      break;
      case 5 :
      {
         if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( fir_input_coefficient ) ) )
            // ���ಾ��ƬO�D FIR ���ɭ�
	    title = "FIR filter derives directly from transfer function";
         else
	    // ���ಾ��ƬO�D IIR ���ɭ�
	    title = "IIR filter derives directly from transfer function";

	                                           break;
      }
      case 6 : title = "protodata filter";          break;
   }

   // �� banner �ܦ� Frequency Sampling = �h���W�v������
   strcat( banner, gtk_entry_get_text( GTK_ENTRY( fs_value ) ) );
   strcat( banner, " HZ" );

   // ��R�O�g�� tmp_script_name �o���ɦW�̭��h�A
   // set terminal postscript �᭱�i�����Ѽƥi�H��landscape, portrait,
   // eps �H�� default �|�ر��ΡA�� eps ���ɭԡA��M�N�O�Ʊ� gnuplot ���X���ϧ�
   // �i?H��X���@�� eps ���ɮסA���L�p�G�A���򳣤��[�]�i�H�A�o�ɭԹ��X�Ӫ��ϧΤ��
   // �j�i�A�[�F eps �o�ӰѼƤ���A�ϦӤp�i�A���~�A�p�G�A�n��X�� ��L���ɮצp�P
   // ps �ɡB�Ϊ̬O pdf �ɡA�]���i�H�����b�x�s���|�W�[�W�һݭn���ɦW�Y�i��X���A
   // �һݭn���榡
   // color �ΤF����A�A�N�|���D����s�����H�ͬO�m�⪺���A���Ϊ��ܡA�A�����H�ʹN�O�¥ժ���
   if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( big_spectrum ) ) )
      fprintf( tmp_script, "set term postscript color" );
   else
      fprintf( tmp_script, "set term postscript eps color" );

   fflush( tmp_script );

   // �o�@�q�N�u�����ӲM���F
   fprintf( tmp_script, " dashed simplex \"Helvetica\" 22\n" );
   fflush( tmp_script );

   if( enable_printer )
   {  //  fprintf( tmp_script, "set output '| lp -d linux1'\n" );
      fprintf( tmp_script, "set output '| lp -d %s'\n",
      gtk_entry_get_text( GTK_ENTRY( printer_name ) ) );
      fflush( tmp_script );
   }
   else
      fprintf( tmp_script, "set output '%s'\n", output_file_name );
   fprintf( tmp_script, "set title '%s'\n", title );
   fprintf( tmp_script, "set xlabel '%s'\n", xlabel );
   fprintf( tmp_script, "set ylabel '%s'\n", ylabel );

   fflush( tmp_script );

   // ��ϧι��X�åB��U�I�s�b�@�_�A�B�U�I����ܡA�� t �i�H�b�k�W����ܦ��o�i���������W�v�A
   // linespoints �O�����u���S�����I��
   // �[�F smooth csplines �O�i�H��u�����ƤƳB�z
   // fprintf( tmp_script, "plot '%s' using 1:2 smooth csplines t '%s' with lines\n", tmp_data_name , banner );

   fprintf( tmp_script, "plot '%s' using 1:2 t '%s' with lines\n", tmp_data_name , banner );
   fflush( tmp_script );

   // ���� tmp_script
   fclose( tmp_script );


/*
   while( b[ i ] != '\0' )
   {
      g_print("the answer's is %f\n",b[i]);
      i++;
   }
*/
   sprintf( sys_command, "gnuplot %s", tmp_script_name );

   if( ( system( sys_command ) ) == -1 )
      gtk_statusbar_push( GTK_STATUSBAR( statusbar ), 1
	 , "��b���� gnuplot ���ɭԡA�����~�o�͡A�Цۦ�ѨM gnuplot �����D��A�A�����W�Ъ�ø�s" );
   else
      gtk_statusbar_push( GTK_STATUSBAR( statusbar ), 1
	 , spectrum_message );

   // ���浲�G��A�ߨ�� script �H�Χ� data ����
   sprintf( sys_command, "rm -f %s %s", tmp_script_name, tmp_data_name );

   // system �o�Ө禡�i�H�N�ѼơA���׺ݾ����U�h����
   system( sys_command );
}

// KIND = 1

void
FIR_Moving_Average( GtkButton *button, gint mode )
{
   // �o�� widget �O�o�Ө禡�һݭn��
   GtkWidget *frequency_sampling = lookup_widget( GTK_WIDGET( button ), "fs" );
   GtkWidget *pass_band_cutoff = lookup_widget( GTK_WIDGET( button ), "pass_edge" );

   //   �H�U�O��{�@��FIR( finite impulse response) ���o�i���A���ͨ��W�йϨ䰵�k
   //   �O�����@�� Moving average ���C�q�o�i���A�A���ڭ̥��氲�]�䶥�Ƭ���11�����A
   //   ���~�A�H�U���{�����A���@�Ǧa�賣�O���Ψ�G�[fs = 2�kf�]�N�O


   //                 �Ʀ��W�v �� �����W�v �� 2 �k �� �����W�v

   //      �q�`�ӻ��A�@�� FIR �o�i�����]�p�A�� window �b��Ϊ��ɭԡA���ӳ��O�ھڨ�
   //   ��a�I��v�]stop band attenuation�^�ӿ�ܪ��ɭԡA�O��̱ܳ��񪺤�a�I��v
   //   �p�G���ݨD���ȶȤj��Y�@�������Ȩǳ\�A�h������ܧ�j��a�I��v�������A��p��
   //   �A�ݨD�n�O46 DB�A�ܩ���a�A�O�ܱ��� Hanning ��45 DB�A�i�O���F�n�F����46 DB
   //   ���ݨD�h�����n�� Hamming �������A�����O�A�Ө� window �����b�U�j DSP
   //   �Ʀ�H���B�z��?��y���ӳ����Ҧ����Ф@�ǡC

   //   �H�U�U�ӰѼƬO�������C
   //    interval �º�O�@�ӹϤW���U�I�bX�y�ФW�����Z

   gint i = 0 ,j = 0;

   // �q�a�I���W�v
   //gfloat cut_off_edge = 480;

   gfloat cut_off_edge = atof( gtk_entry_get_text( GTK_ENTRY( pass_band_cutoff ) ) );


   // �����W�v
   //gfloat fs = 10000;

   gfloat fs = atof( gtk_entry_get_text( GTK_ENTRY( frequency_sampling ) ) );

   //   floor �o�Ө�ƬO����ƭȡA�B�O�L����˥h�p���I�᭱���Ʀr�A��N��N�q??
   //   �b�W��X�y�ФW���ҭn�����ƥءC�ѩ󤺩w���]���a�I��v��40 DB �A
   //   �G��FIR��������A�o�����Өϥ� Hanning �����A�� N ����k��

   //		    N = 3.32 * �����W�v / �ഫ�W�a


   // �Ʀ��W�v �[ �� cut_off_edge ���D�k�A M_PI �O �k �A�b math.h �������w�q
   gfloat omega_cut_off = 2 * M_PI * cut_off_edge / fs;

   gint N = ceil( M_PI / omega_cut_off );

   // �b�W�йϤW�̤j���W�v�ȡA���Ȭ۷�� �[ = �k
   gfloat f_max = fs / 2, omega_max = M_PI ;

   // �o�ӬO�N��q 0 �� f_max�A�Ϊ̻��O 0 �� �[������Ҷ��j���W�v�A
   // ��άO�Q���O�b�W�v X �b�W�������j

   gfloat f_interval = f_max / pixel, omega_interval = omega_max / pixel;

   gfloat f = 0, omega = 0;


   //   �ѤשԦ� Euler's identity �i���A���ƪ������Y���Ƽơ] complex number �^
   //   ���ܫh�i�Ϲ�ƫ�������

   //                     cos(�c) - jsin(�c)

   //   �ӤU�����ܼƫŧi���ҥH�n���}�O�]�� math.h ����Ƥ��S���B�z����ơA�B�䭼����
   //   �Ƽƪ��������A�b���S�O�����곡�M�곡�A�]���b�p���W�q�j�p�] Gain �^���ɭԡA
   //   �u�O��§�곡�]real part�^�A�H�ε곡�]imaginary�^��ӳ����@��������ڪ��B
   //   �z�A�ܩ�Y�O�n��s�ۦ�( phase )���������ܡA�h�n�Ҽ{���S���t�ƪ����D�I

   gfloat H_real = 0, H_imaginary = 0;


   //   ������ܼơA���곡�A�H�ε곡���`�M�O�ǳƥΨӰ��}�ڸ�����M�A�䵲�G�K���W�qGain
   //   |H(�[)|�A�]�N�O�ڭ̪� Y �b�աI

   gfloat sum_real = 0, sum_imaginary = 0;


   //   gain �M sum_real �H�� sum_imaginary ���۳o�˪����Y�G

   //                  gain = ��( sum_real���� + sum_imaginary���� )

   //   gain�N�O|H(�[)|�A�M�� gain_in_db �M gain �O���۳o�˪����Y�G

   //                  gain_in_db = 20*log( gain )

   //   ���N�O�n�����j�p�Adb �O�n�������A�s���� ( decibels )

   gfloat gain, gain_in_db;

/*
   gfloat coefficient[ MAX_VALUE ];

   gfloat *ptr_coefficient = coefficient;
*/
   for( i = 0; i < pixel; i++ )
   {
      for( j = 0; j <= ( N - 1 ); j++ )
      {
         switch( mode )
	 {
	    case 0 :
	    case 1 :
	    case 2 :
            {
	       // ��ܨϥμƦ��W�v��� X �b
               H_real = cos( j * omega );
               H_imaginary = sin( j * omega );
	       break;
	    }

	    case 3 :
	    case 4 :
	    {
               // ��ܨϥ������W�v��� X �b
	       H_real = cos( j * 2 * M_PI * f / fs );
               H_imaginary = sin( j * 2 * M_PI * f / fs );
	       break;
	    }
	 }

	 sum_real += H_real;
         sum_imaginary += H_imaginary;

//	 coefficient[ j ] = 1. / N;
      }

      sum_real = sum_real / N ;
      sum_imaginary = sum_imaginary / N ;

      gain = sqrt( pow( sum_real, 2 ) + pow( sum_imaginary, 2 ) );
      gain_in_db = 20 * log10( gain );

      switch( mode )
      {
         case 0 :
	 case 1 :
	 {
            // ��ܨϥμƦ��W�v��� X �b�A�@��W�q��� Y �b
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI , gain );

	    // X �b���Ʀ��W�v�ɪ��֥[
	    omega= omega + omega_interval;

	    break;
	 }
	 case 2 :
	 {
            // ��ܨϥμƦ��W�v��� X �b�A���q�W�q��� Y �b
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI, gain_in_db );

	    // X �b���Ʀ��W�v�ɪ��֥[
	    omega= omega + omega_interval;

	    break;
	 }
	 case 3 :
	 {
	    // ��ܨϥ������W�v��� X �b�A�@��W�q��� Y �b
            fprintf( tmp_data, "%f \t %f\n", f, gain );

	    // X �b�������W�v�ɪ��֥[
	    f = f + f_interval;

	    break;
	 }
	 case 4 :
	 {
            // ��ܨϥ������W�v��� X �b�A���q�W�q��� Y �b
	    fprintf( tmp_data, "%f \t %f\n", f, gain_in_db );

	    // X �b�������W�v�ɪ��֥[
	    f = f + f_interval;

	    break;
	 }
      }

      fflush( tmp_data );
      sum_real = 0;
      sum_imaginary = 0;
   }

   // �������� tmp_data �n�� gnuplot ���@�U�i�HŪ�o�ӼȦs�������
   fclose( tmp_data );

   // ��Y�ƶǦ^�h
  // return ptr_coefficient;
}

// �� KIND = 2
void
FIR_Window( GtkButton *button, gint mode )
{

   // �o�� widget �O�o�Ө禡�һݭn��
   GtkWidget *frequency_sampling = lookup_widget( GTK_WIDGET( button ), "fs" );
   GtkWidget *pass_band_cutoff = lookup_widget( GTK_WIDGET( button ), "pass_edge" );
   GtkWidget *stop_band_cutoff = lookup_widget( GTK_WIDGET( button ), "stop_edge" );

   //   �H�U�O��{�@��FIR( finite impulse response) ���o�i���A���ͨ��W�йϨ䰵�k
   //   �O�����@�ӥ[�F window ���C�q�o�i���A�A���ڭ̥��氲�]���a�I��ֲv��40db
   //   �p?��@�ӡA��ܺ~�������O��?A���A�Ө��Ҧ����������O�g�礽��(rule of thumb)
   //   ���~�A�H�U���{�����A���@�Ǧa�賣�O���Ψ�G�[fs = 2�kf�]�N�O


   //                �Ʀ��W�v �� �����W�v �� 2 �k �� �����W�v

   //      �q�`�ӻ��A�@�� FIR �o�i�����]�p�A�� window �b��Ϊ��ɭԡA���ӳ��O�ھڨ�
   //   ��a�I��v�]stop band attenuation�^�ӿ�ܪ��ɭԡA�O��̱ܳ��񪺤�a�I��v
   //   �p�G���ݨD���ȶȤj��Y�@�������Ȩǳ\�A�h������ܧ�j��a�I��v�������A��p��
   //   �A�ݨD�n�O46 DB�A�ܩ���a�A�O�ܱ��� Hanning ��45 DB�A�i�O���F�n�F����46 DB
   //   ���ݨD�h�����n�� Hamming �������A�����O�A�Ө� window �����b�U�j DSP
   //   �Ʀ�H���B�z���������y?��ӳ����Ҧ����Ф@�ǡC

   //   �H�U�U�ӰѼƬO�������C
   //    interval �º�O?@�ӹϤW���U�I�bX�y�ФW�����Z

   gint i = 0, j = 0;

   // �q�a���
   //gfloat pass_band_edge = 2000;

   gfloat pass_band_edge = atof( gtk_entry_get_text( GTK_ENTRY( pass_band_cutoff ) ) );

   // ��a���
   //gfloat stop_band_edge = 3000;

   gfloat stop_band_edge = atof( gtk_entry_get_text( GTK_ENTRY( stop_band_cutoff ) ) );

   // �����W�v
   //gfloat fs = 10000;

   gfloat fs = atof( gtk_entry_get_text( GTK_ENTRY( frequency_sampling ) ) );

   //�L���W�a�j�p
   gfloat transition_width = stop_band_edge - pass_band_edge ;

   // �� f_transition_half �ȧY�O�W�ФW�ഫ�a�W���������ȡ�
   gfloat f_transition_half = pass_band_edge + transition_width / 2;


   //   floor �o�Ө�ƬO����ƭȡA�B�O�L����˥h�p���I�᭱���Ʀr�A��N��N�q??
   //   �b�W��X�y�ФW���ҭn�����ƥءC�ѩ󤺩w���]���a�I��v��40 DB �A
   //   �G��FIR��������A�o�����Өϥ� Hanning �����A�� N ����k��

   //		    N = 3.32 * �����W�v / �ഫ�W�a

   gint N = floor( 3.32 * fs / transition_width );

   // �Ʀ��W�v �[ ���D�k�AM_PI �O �k�A�b math.h �������w�q
   gfloat omega_transition_half = 2 * M_PI * f_transition_half / fs;

   //   ���������Y�� h = h1 * w �A�� MAX_VALUE �w�q�b�̤W��

   gfloat h[ MAX_VALUE ], h1, w ;

   // �b�W�йϤW�̤j���W�v�ȡA���Ȭ۷��[ = �k
   gfloat f_max = fs / 2, omega_max = M_PI;

   // �o�ӬO�N��q 0 �� f_max�A�Ϊ̻��O 0 �� �[������Ҷ��j���W�v�A
   // ��άO�Q���O�b�W�v X �b�W�������j

   gfloat f_interval = f_max / pixel, omega_interval = omega_max / pixel;

   gfloat f = 0, omega = 0;

   //   �ѤשԦ� Euler's identity �i���A���ƪ������Y���Ƽơ] complex number �^
   //   ���ܫh�i�Ϲ�ƫ�������

   //                     cos(�c)- jsin(�c)

   //   �ӤU�����ܼƫŧi���ҥH�n���}�O�]�� math.h ����Ƥ��S���B�z����ơA�B�䭼����
   //   �Ƽƪ��������A�b���S�O�����곡�M�곡�A�]���b�p���W�q�j�p�] Gain �^���ɭԡA
   //   �u�O��§�곡�]real part�^�A�H�ε곡�]imaginary�^��ӳ����@��������ڪ��B
   //   �z�A�ܩ�Y�O�n��s�ۦ�( phase )���������ܡA�h�n�Ҽ{���S��?t�ƪ����D�I

   gfloat H_real = 0, H_imaginary = 0;


   //   ������ܼơA���곡�A�H�ε곡���`�M�O�ǳƥΨӰ��}�ڸ�����M�A�䵲�G�K���W�qGain
   //   |H(�[)|�A�]�N�O�ڭ̪� Y �b�աI

   gfloat sum_real = 0, sum_imaginary = 0;


   //   gain �M sum_real �H�� sum_imaginary ���۳o�˪����Y�G

   //                  gain = ��( sum_real���� + sum_imaginary���� )

   //   gain�N�O|H(�[)|�A�M�� gain_in_db �M gain �O���۳o�˪����Y�G

   //                  gain_in_db = 20*log( gain )

   //   ���N�O�n�����j�p�Adb �O�n�������A�s���� ( decibels )

   gfloat gain, gain_in_db;


/*
   gfloat coefficient[ MAX_VALUE ];

   gfloat *ptr_coefficient = coefficient;
*/
   //   �H�U�O�p���Y�ƪ������өҿ׫Y�ơA�K�O h �A�� h ���� w * h1�A���ҥH�u�� i ��
   //   ( N - 1 ) / 2����]�A�O�]���ϧάO�@�ӹ�٪��W�q���ΡA�ҥH�u�n���@�b���ϧδN��
   //   �D�t�@�b�F�I�ӥu�� �k ����]��M�]�����ƬO 2�k ���@�Ӷg���C
   for( i = ( ( N - 1 ) / 2 ) * ( -1 ), j = 0; i <= ( N - 1 ) / 2; i++, j++)
   {
      //�ѤW�������A?~������������
      h1 = sin( omega_transition_half * i ) / ( i * M_PI );
      w = 0.5 + 0.5 * cos( ( 2 * M_PI * i ) / ( N - 1 ) );
      h[ j ] = ( h1 * w );
  //    coefficient[ j ] = h[ j ];
      //printf("%d %f \n", j, h[ j ] );
   }


   //   ���M���s�ȡA���O��M����u���a0�o�ӭȶi�h�A�]�������O���ର 0 ���A�ҥH�a�J�@��
   //   �ܤp���ƥءA�]�N�O 0 = 0.00001 ���N���աI

   h[ ( N - 1 ) / 2 ] = ( sin( omega_transition_half * 0.00001 ) / M_PI / 0.00001 )
            * ( 0.5 + 0.5 * cos( 2 * M_PI * 0.00001 / ( N - 1 ) ) );

   for( i = 0; i < pixel; i++ )
   {
      for( j = 0; j <= ( N - 1 ); j++ )
      {
         switch( mode )
	 {
	    case 0 :
	    case 1 :
	    case 2 :
            {
	       // ��ܨϥμƦ��W�v��� X �b
               H_real = h[ j ] * cos( j * omega );
               H_imaginary = h[ j ] * sin( j * omega );
	       break;
	    }

	    case 3 :
	    case 4 :
	    {
	       // ��ܨϥ������W�v��� X �b
	       H_real = h[ j ] * cos( j * 2 * M_PI * f / fs );
               H_imaginary = h[ j ] * sin( j * 2 * M_PI * f / fs );
               break;
	    }
	 }

         sum_real += H_real;
         sum_imaginary += H_imaginary;
      }

      gain = sqrt( pow( sum_real, 2 ) + pow( sum_imaginary, 2 ) );
      gain_in_db = 20 * log10( gain );

      switch( mode )
      {
         case 0 :
	 case 1 :
	 {
            // ��ܨϥμƦ��W�v��� X �b�A�@��W�q��� Y �b
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI , gain );

	    // X �b���Ʀ��W�v�ɪ��֥[
	    omega= omega + omega_interval;

	    break;
	 }
	 case 2 :
	 {
            // ��ܨϥμƦ��W�v��� X �b�A���q�W�q��� Y �b
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI, gain_in_db );

	    // X �b���Ʀ��W�v�ɪ��֥[
	    omega= omega + omega_interval;

	    break;
	 }
	 case 3 :
	 {
	    // ��ܨϥ������W�v��� X �b�A�@��W�q��� Y �b
            fprintf( tmp_data, "%f \t %f\n", f, gain );

	    // X �b�������W�v�ɪ��֥[
	    f = f + f_interval;

	    break;
	 }
	 case 4 :
	 {
            // ��ܨϥ������W�v��� X �b�A���q�W�q��� Y �b
	    fprintf( tmp_data, "%f \t %f\n", f, gain_in_db );

	    // X �b�������W�v�ɪ��֥[
	    f = f + f_interval;

	    break;
	 }
      }

      fflush( tmp_data );
      sum_real = 0;
      sum_imaginary = 0;
   }
   // �������� tmp_data �n�� gnuplot ���@�U�i�HŪ�o�ӼȦs�������
   fclose( tmp_data );


  // return ptr_coefficient;
}

// KIND = 3
void
IIR_Butterworth( GtkButton *button, gint mode )
{
   // �o�� widget �O�o�Ө禡�һݭn��
   GtkWidget *frequency_sampling = lookup_widget( GTK_WIDGET( button ), "fs" );
   GtkWidget *pass_band_cutoff = lookup_widget( GTK_WIDGET( button ), "pass_edge" );
   GtkWidget *stop_band_cutoff = lookup_widget( GTK_WIDGET( button ), "stop_edge" );
   GtkWidget *stop_band_ripple = lookup_widget( GTK_WIDGET( button ), "stop_ripple" );

   // �H�U���ѼƬO�Ψӫ��w�@�� IIR �o�i�����ԲӳW��A�b���O�Τ@�� Butterworth ���Ҥl
   // �b�s�@�@�ӼƦ� IIR �o�i�����ɭԡA�䭺�n����O�n�������X?@�������o�i���A�M��A��
   // �����u�ഫ�ӧ������ন�Ʀ쪺�o�i��

   // �����W�v
   //gfloat fs = 8000;

   gfloat fs = atof( gtk_entry_get_text( GTK_ENTRY( frequency_sampling ) ) );

   // �����o�i�����q�a�W�v
   //gfloat analog_fp = 1200;

   gfloat analog_fp = atof( gtk_entry_get_text( GTK_ENTRY( pass_band_cutoff ) ) );

   // �����o�i������a�W�v
   //gfloat analog_fs = 1500;

   gfloat analog_fs = atof( gtk_entry_get_text( GTK_ENTRY( stop_band_cutoff ) ) );

   // �Ʀ��o�i�����q�a�W�v
   gfloat digital_omega_p = 2 * M_PI * analog_fp / fs;

   // �Ʀ��o�i������a�W�v
   gfloat digital_omega_s = 2 * M_PI * analog_fs / fs;

   // �g�����u�ഫ������q�a�W�v
   gfloat wp = 2 * fs * tan( digital_omega_p / 2 );

   // �g�����u�ഫ�������a�W�v
   gfloat ws = 2 * fs * tan( digital_omega_s / 2 );

   // ��a���i
   //gfloat delta_s = pow( 10. , ( -25. / 20. ) );

   gfloat delta_s = pow( 10. ,
   ( -( atof( gtk_entry_get_text( GTK_ENTRY( stop_band_ripple ) ) ) ) / 20. ) );

   // �Ʀ��W�v �[ = 0 �� �k�A�۷�������W�v f = 0 �� fs / 2
   gfloat f_max = fs /2, omega_max = M_PI;

   // �Ѱ��w�� pixel ���A�ڭ̥�i�����p��X X ?b���W�����j�N���G f_max / pixel �Aomega_max / pixel
   gfloat f_interval = f_max / pixel, omega_interval = omega_max / pixel;

   // �C�����W�� omega �ȡA�����Y�� omega = omega + omega_interval�A�]�N�O X �b�W���I
   // �C�����W�� f ?ȡA�����Y�� f = f + f_interval�A�]�N�O X �b�W���I
   gfloat f = 0, omega = 0;

   // ���h���p��
   gint n = ceil( log10( 1 / pow( delta_s , 2 ) - 1 ) / ( 2 * log10( ws / wp ) ) );

   // �W�q�A�]�N�O|H(f)|�A �W�q�H�n�������Ӫ��
   gfloat gain, gain_in_db;

   // �b�p��W�q���ɭԡA�䤽���Ӫ��A��?@�����ȩ�t�@���ܼƤ���
   gfloat temp = 0;

   // �Ω�j�骺�֭p
   gint i ;

   // ������ i < pixel - 1 �O�H�]���� tan( �k / 2 ) ���ɭԡA���G�O�L���j�A�Өƹ�W�A�b�� �c �b�ܱ���
   // �k / 2 ���ɭԡA�w�g�O�D�`�j���Ʀr�A�X�G���� float ���d��?A�ҥH���F���h�I�쨺�Ӭɭ��A���n�h��L�O��
   // ���w�����աI
   //for (i = 0; i <= pixel ; i++ )
   //for( i = 0; i < pixel - 1; i++)
   for( i = 0; i < pixel - 4; i++)
   {
      switch( mode )
      {
         case 0 :
	 case 1 :
	 case 2 :
         {
            // ����k�O�H X �b���Ʀ��W�v�ɪ���k
            temp = pow( 2 * fs * tan( omega / 2 ) / wp, 2 * n );

	    break;
	 }
	 case 3 :
	 case 4 :
         {
	    // ����k�O�H X �b�������W�v�ɪ���k
            temp = pow( 2 * fs * tan( M_PI * f / fs ) / wp, 2 * n );

	    break;
	 }
      }

      // �����@�몺��ܼW�q
      gain = 1 / pow( temp + 1, 0.5 );

      // ����?H�n�����j�p��ܼW�q
      gain_in_db = 20 * log10( gain );

      switch( mode )
      {
         case 0 :
	 case 1 :
	 {
            // ��ܨϥμƦ��W�v��� X �b�A�@��W�q��� Y �b
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI , gain );

	    // X �b���Ʀ��W�v�ɪ��֥[
	    omega= omega + omega_interval;

	    break;
	 }
	 case 2 :
	 {
            // ��ܨϥμƦ��W�v��� X �b�A���q�W�q��� Y �b
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI, gain_in_db );

	    // X �b���Ʀ��W�v�ɪ��֥[
	    omega= omega + omega_interval;

	    break;
	 }
	 case 3 :
	 {
	    // ��ܨϥ������W�v��� X �b�A�@��W�q��� Y �b
            fprintf( tmp_data, "%f \t %f\n", f, gain );

	    // X �b�������W�v�ɪ��֥[
	    f = f + f_interval;

	    break;
	 }
	 case 4 :
	 {
            // ��ܨϥ������W�v��� X �b�A���q�W�q��� Y �b
	    fprintf( tmp_data, "%f \t %f\n", f, gain_in_db );

	    // X �b�������W�v�ɪ��֥[
	    f = f + f_interval;

	    break;
	 }
      }

      fflush( tmp_data );

   }

   // �������� tmp_data �n�� gnuplot ���@�U�i�HŪ�o�ӼȦs�������
   fclose( tmp_data );
}

// KIND = 4
void
IIR_Chebyshev( GtkButton *button, gint mode )
{
   // �o�� widget �O�o�Ө禡�һݭn��
   GtkWidget *frequency_sampling = lookup_widget( GTK_WIDGET( button ), "fs" );
   GtkWidget *pass_band_cutoff = lookup_widget( GTK_WIDGET( button ), "pass_edge" );
   GtkWidget *stop_band_cutoff = lookup_widget( GTK_WIDGET( button ), "stop_edge" );
   GtkWidget *pass_band_ripple = lookup_widget( GTK_WIDGET( button ), "pass_ripple" );
   GtkWidget *stop_band_ripple = lookup_widget( GTK_WIDGET( button ), "stop_ripple" );

   // �H�U���ѼƬO�Ψӫ��w�@�� IIR �o�i�����ԲӳW��A�b���O�Τ@�� Chebyshev Type I
   // ���Ҥl�b�s�@�@�Ӽ�?? IIR �o�i�����ɭԡA�䭺�n����O�n�������X�@�������o�i���A
   // �M��A�������u�ഫ�ӧ������ন�Ʀ쪺�o�i��

   // �����W�v
   //gfloat fs = 20000;

   gfloat fs = atof( gtk_entry_get_text( GTK_ENTRY( frequency_sampling ) ) );

   // �����o�i�����q�a�W�v
   //gfloat analog_fp = 5000;

   gfloat analog_fp = atof( gtk_entry_get_text( GTK_ENTRY( pass_band_cutoff ) ) );

   // �����o�i����?�a�W�v
   //gfloat analog_fs = 7500;

   gfloat analog_fs = atof( gtk_entry_get_text( GTK_ENTRY( stop_band_cutoff ) ) );

   // �Ʀ��o�i�����q�a�W�v
   gfloat digital_omega_p = 2 * M_PI * analog_fp / fs;

   // �Ʀ��o�i������a�W�v
   gfloat digital_omega_s = 2 * M_PI * analog_fs / fs;

   // �g�����u�ഫ������q�a�W�v
   gfloat wp = 2 * fs * tan( digital_omega_p / 2 );

   // �g�����u�ഫ�������a�W�v
   gfloat ws = 2 * fs * tan( digital_omega_s / 2 );

   // �q�a���i
   //gfloat delta_p = pow( 10. , ( -1. / 20. ) );

   gfloat delta_p = pow( 10. ,
   ( -( atof( gtk_entry_get_text( GTK_ENTRY( pass_band_ripple ) ) ) )/ 20. ) );

   // �ѳq�a���i�ҨM�w���Ѽƣ`
   gfloat epsilon = sqrt( pow( delta_p, -2 ) - 1 );

   // Chebyshev polynomial ���񳷤Ҧh��������
   gfloat chebyshev;

   // ��a���i�_
   // gfloat delta_s = pow( 10. , ( -32. / 20. ) );
   gfloat delta_s = pow( 10. ,
   ( -( atof( gtk_entry_get_text( GTK_ENTRY( stop_band_ripple ) ) ) ) / 20. ) );

   // �Ѥ�a���i�_�ҨM�w���Ѽƣ_
   gfloat delta = sqrt( pow( delta_s, -2 ) - 1 );

   // �Ʀ��W�v �[ = 0 �� �k�A�۷�������W�v f = 0 �� fs / 2
   gfloat omega_max = M_PI, f_max = fs / 2;

   // �Ѱ��w�� pixel ���A�ڭ̥�i�����p��X X �b���W�����j�N���G omega_max / pixel �Af_max / pixel
   gfloat omega_interval = omega_max / pixel, f_interval = f_max / pixel;

   // �C�����W�� omega �ȡA�����Y�� omega = omega + omega_interval�A�]�N�O X �b�W���I
   // �C�����W�� f �ȡA�����Y�� f = f + f_interval�A�]�N�O X �b�W���I
   gfloat omega = 0, f = 0;

   // ���h���p��
   gint n = ceil( acosh( delta / epsilon ) / acosh( ws / wp ) );

   // �W�q�A�]�N�O|H(f)|, �W�q�H�n�������Ӫ��
   gfloat gain, gain_in_db;

   // �b�p��W�q���ɭԡA�䤽���Ӫ��A��@�����ȩ�t�@�Ӱ}�C����
   gfloat temp = 0;

   // �Ω�j�骺�֭p
   gint i ;

   // �Ʀ��o�i���b�W�ФW�� X �� Y ���D�ȫ�A�g�J�ɮ׫��� tmp_data �����A

   // ������ i < pixel - 1 �O�H�]���� tan( �k / 2 ) ���ɭԡA���G�O�L���j�A�Өƹ�W�A�b�� �c �b�ܱ���
   // �k / 2 ���ɭԡA�w�g�O�D�`�j���Ʀr�A�X�G���� float ���d��A�ҥH���F���h�I�쨺�Ӭɭ��A���n�h��L�O��
   // ���w�����աI
   //for( i = 0; i <= pixel; i++)
   for( i = 0; i < pixel - 1; i++)
   {
      switch( mode )
      {
         case 0 :
	 case 1 :
	 case 2 :
         {
            // ����k�O�H X �b���Ʀ��W�v�ɪ���k
            temp = 2 * fs * tan( omega / 2 ) / wp;

	    break;
	 }
	 case 3 :
	 case 4 :
         {
            // �� X �O�H�������W�v�����
            temp = 2 * fs * tan( M_PI * f / fs ) / wp;

	    break;
	 }
      }
      // �]�� abs �o�Ө禡�u��Φb��ƪ��ѼơA�ҥH�L�k�b���ϥ�
      if( temp < 0 )
         temp = temp * ( -1 ) ;

      // �U�����P�_���N�O���񳷤Ҫ��h�����A�� temp ������Ȥj��1�ɡA�M�p��1�ɪ����P
      if( temp > 1. )
         chebyshev = cosh( n * acosh( temp ) );
      else
         chebyshev =  cos( n *  acos( temp ) );

      // �����@�몺��ܼW�q
      gain = pow( 1 + pow( epsilon, 2 ) * pow( chebyshev, 2 ) , -0.5 );

      // �����H�n�����j�p��ܼW�q
      gain_in_db = 20 * log10( gain );

      switch( mode )
      {
         case 0 :
	 case 1 :
	 {
            // ��ܨϥμƦ��W�v��� X �b�A�@��W�q��� Y �b
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI , gain );

	    // X �b���Ʀ��W�v�ɪ��֥[
	    omega= omega + omega_interval;

	    break;
	 }
	 case 2 :
	 {
            // ��ܨϥμƦ��W�v��� X �b�A���q�W�q��� Y �b
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI, gain_in_db );

	    // X �b���Ʀ��W�v�ɪ��֥[
	    omega= omega + omega_interval;

	    break;
	 }
	 case 3 :
	 {
	    // ��ܨϥ������W�v��� X �b�A�@��W�q��� Y �b
            fprintf( tmp_data, "%f \t %f\n", f, gain );

	    // X �b�������W�v�ɪ��֥[
	    f = f + f_interval;

	    break;
	 }
	 case 4 :
	 {
            // ��ܨϥ������W�v��� X �b�A���q�W�q��� Y �b
	    fprintf( tmp_data, "%f \t %f\n", f, gain_in_db );

	    // X �b�������W�v�ɪ��֥[
	    f = f + f_interval;

	    break;
	 }
      }

      fflush( tmp_data );
   }

   // �������� tmp_data �n�� gnuplot ���@�U�i�HŪ�o�ӼȦs�������
   fclose( tmp_data );

}

// KIND = 5
void
Transfer_function( GtkButton *button, gint mode )
{
   GtkWidget *saveaspath = lookup_widget( GTK_WIDGET( button ), "entry1" );

   GtkWidget *frequency_sampling = lookup_widget( GTK_WIDGET( button ), "fs" );
   GtkWidget *iir_input_coefficient = lookup_widget( GTK_WIDGET( button ), "iir_input_coefficient" );
   GtkWidget *a_value = lookup_widget( GTK_WIDGET( button ), "a_value" );
   GtkWidget *b_value = lookup_widget( GTK_WIDGET( button ), "b_value" );


   //gfloat fs = 10000;
   gfloat fs = atof( gtk_entry_get_text( GTK_ENTRY( frequency_sampling ) ) );

   // �b�W�йϤW�̤j���W�v�ȡA���Ȭ۷�� �[ = �k
   gfloat f_max = fs / 2, omega_max = M_PI;

   // �o�ӬO�N��q 0 �� f_max�A�Ϊ̻��O 0 �� �[������Ҷ��j���W�v�A
   // ��άO�Q���O�b�W�v X �b�W�������j

   gfloat f_interval = f_max / pixel, omega_interval = omega_max / pixel;

   gfloat f = 0, omega = 0;

   //   �ѤשԦ� Euler's identity �i���A���ƪ������Y���Ƽơ] complex number �^
   //   ���ܫh�i�Ϲ�ƫ�������

   //                     cos(�c)- jsin(�c)

   //   �ӤU�����ܼƫŧi���ҥH�n���}�O�]�� math.h ����Ƥ��S���B�z����ơA�B�䭼����
   //   �Ƽƪ��������A�b���S�O��?��곡�M�곡�A�]���b�p���W�q�j�p�] Gain �^���ɭԡA
   //   �u�O��§�곡�]real part�^�A�H�ε곡�]imaginary�^��ӳ����@��������ڪ�?B
   //   �z�A�ܩ�Y�O�n��s�ۦ�( phase )���������ܡA�h�n�Ҽ{���S��?t�ƪ����D�I

   gfloat H_real_a = 0, H_imaginary_a = 0, H_real_b = 0, H_imaginary_b = 0;


   //   ������ܼơA���곡�A�H�ε곡���`�M�O�ǳƥΨӰ��}�ڸ�����M�A�䵲�G�K���W�qGain
   //   |H(�[)|�A�]�N�O�ڭ̪� Y �b�աI

   gfloat sum_real_a = 0, sum_imaginary_a = 0, sum_real_b = 0, sum_imaginary_b = 0;

   //   gain �M sum_real �H�� sum_imaginary ���۳o�˪����Y�G

   //                  gain = ��( sum_real���� + sum_imaginary���� )

   //   gain�N�O|H(�[)|�A�M�� gain_in_db �M gain �O���۳o�˪����Y�G

   //                  gain_in_db = 20*log( gain )

   //   ���N�O�n�����j�p�Adb �O�n�������A�s���� ( decibels )

   gfloat gain, gain_in_db;

   // �Ψө�n�J�s��ƪ����|�ɦW
   gchar *path;

   // �N�O�@�ӭn�Q�g�J����ɪ��ɦW����
   FILE *file = NULL;

   gchar str[80];

   // �P�_�O�_�����B�z����ơATRUE �h�����O��
   gboolean crude = FALSE;

   // �]���i�঳�H����ƹ�b�j��E�Q�E�U�A�ҥH�@�w�n�� long
   glong raw_data[ MAX_VALUE ],ripe_data[ MAX_VALUE ];

   // �]���ƭȸ���`�[����ܦ��i��|�W�L int ���ɭ��A�ҥH�@�w�n��  gulong
   gulong sum_raw = 0, sum_ripe = 0;

   gfloat average_raw, average_ripe;

   // ������ϥΪ̥i�H�Ρ�,���B��;���Ϊ̬O���ť��䡨�Ӥ��j�ಾ��Ƥ����Y�ơA
   // �� segment �N��Q���Ϊ��U�ӳ����M��A�@�@�s��b�U�Ӱ}�C��m
   // �� substitute �O�ȮɱN entry ���Y�Ƽȩ�b���B
   gchar *sift = ",; ", *segment = "", substitute[ MAX_VALUE ];

   // term �N���ơA�]�N�O�X�����N��A�]�O�Y�ƪ��ƥذաI
   gint term_a = 0, term_b = 0;

   // �i�H��m�Y�ƪ��}�C
   gfloat a_factor[ MAX_VALUE ], b_factor[ MAX_VALUE ];

   gint i = -1, j = -1;
//////////////////////////////
//float data_xn[ MAX_VALUE ] = { 10, 14, 16, 15, 11, 14, 12 };

//float sum_xn = 0, average_xn, sum_yn = 0, average_yn;

gfloat sum_yn = 0, average_yn;
   // �C�����
gint amount = 499;

gfloat yn[ MAX_VALUE ], xn[ MAX_VALUE ];
////////////////////////////////
   system( "clear" );


   // path ��ۭn�g�J������ɸ��|�Ψ��ɮ�
   path = gtk_entry_get_text( GTK_ENTRY( saveaspath ) );

   file = fopen( path, "r" );

   // �T�O�ɮת�Ū�g��m�T��b���Y����m
   rewind( file );

   while( fgets( str, SIZE, file ) )
   {
      // ��ܥ洫���yŪ�����͡���ƤΡ��w�o�i������ƨåB��Ū�Ӫ���Ʃ�J�}�C����
      if( ( crude = !crude ) )
         raw_data[ ++i ] = atoi( str );
      else
         ripe_data[ ++j ] = atoi( str );

      // �o�Ө禡�O��ثe�ҫ����ɮ�Ū�g��m�̡��۹��m�����覡�A�����X��?줸�աA�H�K���U�@�Ӹ��Ū��
      fseek( file, 1, SEEK_CUR );
   }

   // �Χ��N�ߨ����_�ӡA�T�O�w��
   fclose( file );

 //  for( i = 0 ; i<=499; i++)
   {
  //   g_print("%d raw_data = %ld \t ripe_data = %ld \n",i,raw_data[ i ],ripe_data[ i ]);
  //   getchar();
   }

   // ���ɸ�ƪ��`�Ʀ۵M�N�i�H�T�w�F
   amount = i;

   // ��X��l��ƪ��`�M
   for( i = 0; i <= amount; i++)
   {
      sum_raw = sum_raw + raw_data[ i ];
      sum_ripe = sum_ripe + ripe_data[ i ];
   }

//   g_print("sum_raw = %ld \t sum_ripe = %ld \n",sum_raw,sum_ripe );

   // ��X�����ȡA�ഫ�� float �~�i�H���p�ƥX�{�C
   average_raw  = ( gfloat )sum_raw  / ( gfloat )( amount + 1 );
   average_ripe = ( gfloat )sum_ripe / ( gfloat )( amount + 1 );

//   g_print("average_raw = %f \t average_ripe = %f \n",average_raw,average_ripe );


   // �T�w�O��� IIR ���o�i���A�p�G�O�A�h�|�ݭn���Ψ� a ���Y�ơA�� b ���Y�ƬO���ޭ��@��
   // �o�i�����@�w�n���աI
   if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( iir_input_coefficient ) ) )
   {
      // �������� gtk_entry_get_text( GTK_ENTRY( a_value ) ) �@ strtok ���Ѫ��ʧ@
      // ���]�O�]���A�ڵoı��{������ entry �ȥu���b�Q�����ܼƭȡ����ɭԡA��
      // gtk_entry_get_text( GTK_ENTRY( a_value ) ) �~�|�o��t�@�ӷs���ȡA
      // ���y�ܻ��A�]�N�O���`���ɭԹq���B�@���ɭԬO�� gtk_entry_get_text( GTK_ENTRY( a_value ) )
      // �Ȯɩ��Y�ӰO����ϡA�p�G�A����L�S���b���� entry �̭��y���������� key press ���ʧ@���ܡA
      // �ƹ�W�A�b gtk_entry_get_text( GTK_ENTRY( a_value ) ) ���O����A���M�O������
      // �s�Ϫ��ȡA�]�N�O�S���u���h�� entry �̭����ȰաI
      // �ӥ��{�����p�G�S���h���� entry �̭����ȡA�Ӧ]���Ψ� strtok �o�Ө禡�A�ҥH�L�u�|�@���h��
      // �Ȧs�Ϫ��ȡA���F�קK�o�ر��ΡA�N���M�w��m�b�@�� substitute �o�Ӱ}�C���A
      strcpy( substitute, gtk_entry_get_text( GTK_ENTRY( a_value ) ) );

      a_factor[ term_a ] = atof( strtok( substitute, sift ) );

      while( ( segment = strtok( NULL, sift ) ) )
      {
         term_a++;
         a_factor[ term_a ] = atof( segment );
      }
   }
   // g_print("term_a = %d\n", term_a );

   // �@�k�Ψ�z�ѩM a_value �@��
   strcpy( substitute, gtk_entry_get_text( GTK_ENTRY( b_value ) ) );
   b_factor[ term_b ] = atof( strtok( substitute , sift ) );

   while( ( segment = strtok( NULL, sift ) ) )
   {
      term_b++;
      b_factor[ term_b ] = atof( segment );
   }
   // g_print("term_b = %d\n", term_b );

////////////////////////
   // ��X��l data ���`�M
  // for( i = 0; i <= amount; i++)
  //    sum_xn = sum_xn + data_xn[ i ];
   //g_print("sum_xn = %f\n", sum_xn );

   // ��X������
//   average_xn = sum_xn  / ( amount + 1 );
   //g_print("average_xn = %f\n", average_xn );

   // ��n�p�⪺ xn �Ȱ��@�Ӫ�l�ƪ��ʧ@
   for( i = 0; i <= term_b ; i++ )
      xn[ i ] = average_raw;

   xn[ 0 ] = raw_data[ 0 ];

   // ���h�ֿ�J�A�N���h�ֿ�X
   for( i = 0; i <= amount; i++ )
   {
      // ���M���b�A�H���w��
      yn[ i ] = 0;

      // �˵۰��^�ӡA�]�N�O�q�̫�@���}�l���A��Ĥ@��
      for( j = term_b; j >= 0; j-- )
      {
         // �U�����֥[
         yn[ i ] = yn[ i ] + b_factor[ j ] * xn[ j ];

         if( j == 0 )
            // �N��w�g���Ĥ@���F�A��ܦ�������X�Y�N����
	    xn[ j ] = raw_data[ i + 1 ];
	 else
	    // �C�����n�V�k���ʤ@��
	    xn[ j ] = xn[ j - 1 ];
      }

      sum_yn = sum_yn + yn[ i ];
      //g_print("y[%d] = %f\n",i, yn[ i ] );
   }

   // ��X������
   average_yn = sum_yn  / ( amount + 1 );
   // printf("%f\n",average_yn );

////////////

   // �}�l�p�� X �b���U�I
   for( i = 0; i < pixel; i++ )
   {

      // �P�_�O�_�ݭn�Ψ� a �Y��
      if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( iir_input_coefficient ) ) )
      {
         for( j = 0; j <= term_a ; j++ )
         {
            switch( mode )
	    {
	       case 0 :
	       case 1 :
	       case 2 :
               {
	          // ��ܨϥμƦ��W�v��� X �b
                  H_real_a = a_factor[ j ] * cos( j * omega );
                  H_imaginary_a = a_factor[ j ] * sin( j * omega );
	          break;
	       }

	       case 3 :
	       case 4 :
	       {
	          // ��ܨϥ������W�v��� X �b
                  H_real_a = a_factor[ j ] * cos( j * 2 * M_PI * f / fs );
                  H_imaginary_a = a_factor[ j ] * sin( j * 2 * M_PI * f / fs );
                  break;
	       }
	    }

	    sum_real_a += H_real_a;
            sum_imaginary_a += H_imaginary_a;
         }
      }

      for( j = 0; j <= amount ; j++ )
      //for( j = 0; j <= term_b ; j++ )
      {

//	 switch( mode )
	 {
//	    case 0 :
//	    case 1 :
//	    case 2 :
            {
	       // ��ܨϥμƦ��W�v��� X �b
//               H_real_b = b_factor[ j ] * cos( j * omega );
//               H_imaginary_b = b_factor[ j ] * sin( j * omega );
//	       break;
	    }

//            case 3 :
//	    case 4 :
	    {
	       // ��ܨϥ������W�v��� X �b
//               H_real_b = b_factor[ j ] * cos( j * 2 * M_PI * f / fs );
//               H_imaginary_b = b_factor[ j ] * sin( j * 2 * M_PI * f / fs );
//               break;
	    }
	 }


	 switch( mode )
	 {
	    case 0 :
	    case 1 :
	    case 2 :
            {
	       // ��ܨϥμƦ��W�v��� X �b
               H_real_b = ( yn[ j ] - average_yn ) * cos( j * omega );
               H_imaginary_b = ( yn[ j ] - average_yn ) * sin( j * omega );
	       break;
	    }

            case 3 :
	    case 4 :
	    {
	       // ��ܨϥ������W�v��� X �b
	       H_real_b = ( yn[ j ] - average_yn ) * cos( j * 2 * M_PI * f / fs );
               H_imaginary_b = ( yn[ j ] - average_yn ) * sin( j * 2 * M_PI * f / fs );
               break;
	    }
	 }

	 sum_real_b += H_real_b;
         sum_imaginary_b += H_imaginary_b;
      }


      if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( iir_input_coefficient ) ) )
         gain = sqrt( pow( sum_real_b, 2 ) + pow( sum_imaginary_b, 2 ) )
                / sqrt( pow( sum_real_a, 2 ) + pow( sum_imaginary_a, 2 ) );
      else
         gain = sqrt( pow( sum_real_b, 2 ) + pow( sum_imaginary_b, 2 ) );

      gain_in_db = 20 * log10( gain );

      switch( mode )
      {
         case 0 :
	 case 1 :
	 {
            // ��ܨϥμƦ��W�v��� X �b�A�@��W�q��� Y �b
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI , gain );

	    // X �b���Ʀ��W�v�ɪ��֥[
	    omega= omega + omega_interval;

	    break;
	 }
	 case 2 :
	 {
            // ��ܨϥμƦ��W�v��� X �b�A���q�W�q��� Y �b
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI, gain_in_db );

	    // X �b���Ʀ��W�v�ɪ��֥[
	    omega= omega + omega_interval;

	    break;
	 }
	 case 3 :
	 {
	    // ��ܨϥ������W�v��� X �b�A�@��W�q��� Y �b
            fprintf( tmp_data, "%f \t %f\n", f, gain );

	    // X �b�������W�v�ɪ��֥[
	    f = f + f_interval;

	    break;
	 }
	 case 4 :
	 {
            // ��ܨϥ������W�v��� X �b�A���q�W�q��� Y �b
	    fprintf( tmp_data, "%f \t %f\n", f, gain_in_db );

	    // X �b�������W�v�ɪ��֥[
	    f = f + f_interval;

	    break;
	 }
      }

      fflush( tmp_data );

      sum_real_a = 0;
      sum_imaginary_a = 0;
      sum_real_b = 0;
      sum_imaginary_b = 0;

   }

   // �������� tmp_data �n�� gnuplot ���@�U�i�HŪ�o�ӼȦs�������
   fclose( tmp_data );

}


//KIND = 6
void
protodata( GtkButton *button, gint mode )
{
   GtkWidget *saveaspath = lookup_widget( GTK_WIDGET( button ), "entry1" );
   GtkWidget *frequency_sampling = lookup_widget( GTK_WIDGET( button ), "fs" );

   // �Ψө�n�J�s��ƪ����|�ɦW
   gchar *path;

   gint i = -1;

   gint j = -1;

   gint amount;

   // �]���ƭȸ���`�[����ܦ��i��|�W�L int ���ɭ��A�ҥH�@�w�n��  gulong
   gulong sum_raw = 0, sum_ripe = 0;

   gfloat average_raw, average_ripe;

   gfloat fs = atof( gtk_entry_get_text( GTK_ENTRY( frequency_sampling ) ) );

   // �b�W�йϤW�̤j���W�v�ȡA���Ȭ۷�� �[ = �k
   gfloat f_max = fs / 2, omega_max = M_PI ;

   // �o�ӬO�N��q 0 �� f_max�A�Ϊ̻��O 0 �� �[������Ҷ��j���W�v�A
   // ��άO�Q���O�b�W�v X �b�W�������j

   gfloat f_interval = f_max / pixel, omega_interval = omega_max / pixel;

   gfloat f = 0, omega = 0;


   //   �ѤשԦ� Euler's identity �i���A���ƪ������Y���Ƽơ] complex number �^
   //   ���ܫh�i�Ϲ�ƫ�������

   //                     cos(�c) - jsin(�c)

   //   �ӤU�����ܼƫŧi���ҥH�n���}�O�]�� math.h ����Ƥ��S���B�z����ơA�B�䭼����
   //   �Ƽƪ��������A�b���S�O�����곡�M�곡�A�]���b�p���W�q�j�p�] Gain �^���ɭԡA
   //   �u�O��§�곡�]real part�^�A�H�ε곡�]imaginary�^��ӳ����@��������ڪ��B
   //   �z�A�ܩ�Y�O�n��s�ۦ�( phase )���������ܡA�h�n�Ҽ{���S���t�ƪ����D�I

   gfloat H_real = 0, H_imaginary = 0;

   //   ������ܼơA���곡�A�H�ε곡���`�M�O�ǳƥΨӰ��}�ڸ�����M�A�䵲�G�K���W�qGain
   //   |H(�[)|�A�]�N�O�ڭ̪� Y �b�աI

   gfloat sum_real = 0, sum_imaginary = 0;


   //   gain �M sum_real �H�� sum_imaginary ���۳o�˪����Y�G

   //                  gain = ��( sum_real���� + sum_imaginary���� )

   //   gain�N�O|H(�[)|�A�M�� gain_in_db �M gain �O���۳o�˪����Y�G

   //                  gain_in_db = 20*log( gain )

   //   ���N�O�n�����j�p�Adb �O�n�������A�s���� ( decibels )

   gfloat gain, gain_in_db;

   // �N�O�@�ӭn�Q�g�J����ɪ��ɦW����
   FILE *file = NULL;

   gchar str[80];

   // �]���i�঳�H����ƹ�b�j��E�Q�E�U�A�ҥH�@�w�n�� long
   glong raw_data[ MAX_VALUE ],ripe_data[ MAX_VALUE ];

   // �P�_�O�_�����B�z����ơATRUE �h�����O��
   gboolean crude = FALSE;

   // �W�йϤ����`�ȡA�Y�O���ƫר������@�ˡA�h�ȷ|���ܳ�I
   total = 0;

   // �W�йϤ����̤j��
   max = 0;

   // path ��ۭn�g�J������ɸ��|�Ψ��ɮ�
   path = gtk_entry_get_text( GTK_ENTRY( saveaspath ) );

   file = fopen( path, "r" );

   // �T�O�ɮת�Ū�g��m�T��b���Y����m
   rewind( file );

   while( fgets( str, SIZE, file ) )
   {
      // ��ܥ洫���yŪ�����͡���ƤΡ��w�o�i������ƨåB��Ū�Ӫ���Ʃ�J�}�C����
      if( ( crude = !crude ) )
         raw_data[ ++i ] = atoi( str );
      else
         ripe_data[ ++j ] = atoi( str );

      // �o�Ө禡�O��ثe�ҫ����ɮ�Ū�g��m�̡��۹��m�����覡�A�����X��?줸�աA�H�K���U�@�Ӹ��Ū��
      fseek( file, 1, SEEK_CUR );
   }

   // �Χ��N�ߨ����_�ӡA�T�O�w��
   fclose( file );

  // for( i = 0 ; i<=499; i++)
  //   g_print("raw_data = %ld \t ripe_data = %ld \n",raw_data[ i ],ripe_data[ i ]);

   // ���ɸ�ƪ��`�Ʀ۵M�N�i�H�T�w�F
   amount = i;

   // ��X��l��ƪ��`�M
   for( i = 0; i <= amount; i++)
   {
      sum_raw = sum_raw + raw_data[ i ];
      sum_ripe = sum_ripe + ripe_data[ i ];
   }

   //g_print("sum_raw = %ld \t sum_ripe = %ld \n",sum_raw,sum_ripe );

   // ��X�����ȡA�ഫ�� float �~�i�H���p�ƥX�{�C
   average_raw  = ( gfloat )sum_raw  / ( gfloat )( amount + 1 );
   average_ripe = ( gfloat )sum_ripe / ( gfloat )( amount + 1 );

   //g_print("average_raw = %f \t average_ripe = %f \n",average_raw,average_ripe );

   for( i = 0; i < pixel; i++ )
   {
      for( j = 0; j <= amount ; j++ )
      {
         switch( mode )
	 {
	    case 0 :
	    case 1 :
	    case 2 :
            {
	       // ��ܨϥμƦ��W�v��� X �b

      //         H_real = ( raw_data[ j ] - average_raw ) * cos( j * omega );
     //          H_imaginary = ( raw_data[ j ] - average_raw ) * sin( j * omega );

               H_real = ( ripe_data[ j ] - average_ripe ) * cos( j * omega );
               H_imaginary = ( ripe_data[ j ] - average_ripe ) * sin( j * omega );
	       break;
	    }

	    case 3 :
	    case 4 :
	    {
               // ��ܨϥ������W�v��� X �b

	//       H_real = ( raw_data[ j ] - average_raw) * cos( j * 2 * M_PI * f / fs );
    //           H_imaginary = ( raw_data[ j ] - average_raw ) * sin( j * 2 * M_PI * f / fs );

               H_real = ( ripe_data[ j ] - average_ripe) * cos( j * 2 * M_PI * f / fs );
               H_imaginary = ( ripe_data[ j ] - average_ripe ) * sin( j * 2 * M_PI * f / fs );
	       break;
	    }
	 }

	 sum_real += H_real;
         sum_imaginary += H_imaginary;
      }

      gain = sqrt( pow( sum_real, 2 ) + pow( sum_imaginary, 2 ) );
      gain_in_db = 20 * log10( gain );

      // �@�[�`�W�q�Ψ��̤j�Ȫ��ʧ@
      switch( mode )
      {
         case 0 :
	 case 1 :
	 case 3 :
         {
            // �W�йϤW���[�`��
            total = total + gain;

            // �D�o�̤j���u�ʼW�q��
            if( max < gain )
               max = gain;
	    break;
	 }

	 case 2 :
	 case 4 :
	 {
            // �W�йϤW���[�`��
            total = total + gain_in_db;

            // �D�o�̤j�������W�q��
            if( max < gain_in_db )
               max = gain_in_db;
            break;
	 }
      }

      // �ϥ������W�v��� X �b
      switch( mode )
      {
         case 0 :
	 case 1 :
	 {
            // ��ܨϥμƦ��W�v��� X �b�A�@��W�q��� Y �b
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI , gain );

	    // X �b���Ʀ��W�v�ɪ��֥[
	    omega= omega + omega_interval;

	    break;
	 }
	 case 2 :
	 {
            // ��ܨϥμƦ��W�v��� X �b�A���q�W�q��� Y �b
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI, gain_in_db );

	    // X �b���Ʀ��W�v�ɪ��֥[
	    omega= omega + omega_interval;

	    break;
	 }
	 case 3 :
	 {
	    // ��ܨϥ������W�v��� X �b�A�@��W�q��� Y �b
            fprintf( tmp_data, "%f \t %f\n", f, gain );

	    // X �b�������W�v�ɪ��֥[
	    f = f + f_interval;

	    break;
	 }
	 case 4 :
	 {
            // ��ܨϥ������W�v��� X �b�A���q�W�q��� Y �b
	    fprintf( tmp_data, "%f \t %f\n", f, gain_in_db );

	    // X �b�������W�v�ɪ��֥[
	    f = f + f_interval;

	    break;
	 }
      }

      fflush( tmp_data );
      sum_real = 0;
      sum_imaginary = 0;
   }

  // g_print("total = %f ,max = %f\n", total ,max);

   // �������� tmp_data �n�� gnuplot ���@�U�i�HŪ�o�ӼȦs�������
   fclose( tmp_data );


}


void
on_save_as_eps_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
   gchar *a = "eps";
   on_save_as_numeral_clicked( GTK_BUTTON( button ), a );
}


void
on_specification_on_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
   // �����o�i������J����
   GtkWidget *move_average = lookup_widget( GTK_WIDGET( button ), "move_average");
   GtkWidget *window = lookup_widget( GTK_WIDGET( button ),       "window"      );
   GtkWidget *butterworth = lookup_widget( GTK_WIDGET( button ),  "butterworth" );
   GtkWidget *chebyshev = lookup_widget( GTK_WIDGET( button ),    "chebyshev"   );
   GtkWidget *pass_edge   = lookup_widget( GTK_WIDGET( button ),   "pass_edge" );
   GtkWidget *stop_edge   = lookup_widget( GTK_WIDGET( button ),   "stop_edge" );
   GtkWidget *pass_ripple = lookup_widget( GTK_WIDGET( button ), "pass_ripple" );
   GtkWidget *stop_ripple = lookup_widget( GTK_WIDGET( button ), "stop_ripple" );

   // �n�ΰ��]���ಾ��ƫY�ƿ�J������
   GtkWidget *fir_input_coefficient = lookup_widget( GTK_WIDGET( button ), "fir_input_coefficient" );
   GtkWidget *iir_input_coefficient = lookup_widget( GTK_WIDGET( button ), "iir_input_coefficient" );
   GtkWidget *a_value = lookup_widget( GTK_WIDGET( button ), "a_value" );
   GtkWidget *b_value = lookup_widget( GTK_WIDGET( button ), "b_value" );

   // �����o�i��������������}
   gtk_widget_set_sensitive( GTK_WIDGET( move_average ), TRUE );
   gtk_widget_set_sensitive( GTK_WIDGET( window       ), TRUE );
   gtk_widget_set_sensitive( GTK_WIDGET( butterworth  ), TRUE );
   gtk_widget_set_sensitive( GTK_WIDGET( chebyshev    ), TRUE );
   gtk_widget_set_sensitive( GTK_WIDGET( pass_edge ), TRUE );
   gtk_widget_set_sensitive( GTK_WIDGET( stop_edge ), TRUE );
   gtk_widget_set_sensitive( GTK_WIDGET( pass_ripple ), TRUE );
   gtk_widget_set_sensitive( GTK_WIDGET( stop_ripple ), TRUE );

   // ���]���ಾ��ƫY�ƿ�J������A��������
   gtk_widget_set_sensitive( GTK_WIDGET( fir_input_coefficient ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( iir_input_coefficient ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( a_value ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( b_value ), FALSE );

   KIND = 4;
}


void
on_coefficient_on_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{

   // �����o�i������J����
   GtkWidget *move_average = lookup_widget( GTK_WIDGET( button ), "move_average");
   GtkWidget *window = lookup_widget( GTK_WIDGET( button ),       "window"      );
   GtkWidget *butterworth = lookup_widget( GTK_WIDGET( button ),  "butterworth" );
   GtkWidget *chebyshev = lookup_widget( GTK_WIDGET( button ),    "chebyshev"   );
   GtkWidget *pass_edge   = lookup_widget( GTK_WIDGET( button ),   "pass_edge" );
   GtkWidget *stop_edge   = lookup_widget( GTK_WIDGET( button ),   "stop_edge" );
   GtkWidget *pass_ripple = lookup_widget( GTK_WIDGET( button ), "pass_ripple" );
   GtkWidget *stop_ripple = lookup_widget( GTK_WIDGET( button ), "stop_ripple" );

   // �n�ΰ��]���ಾ��ƫY�ƿ�J������
   GtkWidget *fir_input_coefficient = lookup_widget( GTK_WIDGET( button ), "fir_input_coefficient"      );
   GtkWidget *iir_input_coefficient = lookup_widget( GTK_WIDGET( button ), "iir_input_coefficient" );
   GtkWidget *a_value = lookup_widget( GTK_WIDGET( button ), "a_value" );
   GtkWidget *b_value = lookup_widget( GTK_WIDGET( button ), "b_value" );

   // �����o�i���������������
   gtk_widget_set_sensitive( GTK_WIDGET( move_average ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( window       ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( butterworth  ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( chebyshev    ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( pass_edge ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( stop_edge ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( pass_ripple ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( stop_ripple ), FALSE );

   // ���]���ಾ��ƫY�ƿ�J������A�����P��
   gtk_widget_set_sensitive( GTK_WIDGET( fir_input_coefficient ), TRUE );
   gtk_widget_set_sensitive( GTK_WIDGET( iir_input_coefficient ), TRUE );
   gtk_widget_set_sensitive( GTK_WIDGET( a_value ), TRUE );
   gtk_widget_set_sensitive( GTK_WIDGET( b_value ), TRUE );



   statusbar = lookup_widget( GTK_WIDGET( button ), "statusbar1" );

   // ��T����J���A�C��
   gtk_statusbar_push( GTK_STATUSBAR( statusbar ), 1, "�ХΡ��ť��䡨�Ρ�;����,�����j�Y�ơA�Y���ʶ��A�нЦۦ�ɡ�0��" );

   // ���]���ಾ��ƪ��p�ⳣ�b KIND = 5 �̭��p��X��
   KIND = 5;
}

void
on_move_average_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
   GtkWidget *fs          = lookup_widget( GTK_WIDGET( button ),   "fs"        );
   GtkWidget *pass_edge   = lookup_widget( GTK_WIDGET( button ),   "pass_edge" );
   GtkWidget *stop_edge   = lookup_widget( GTK_WIDGET( button ),   "stop_edge" );
   GtkWidget *pass_ripple = lookup_widget( GTK_WIDGET( button ), "pass_ripple" );
   GtkWidget *stop_ripple = lookup_widget( GTK_WIDGET( button ), "stop_ripple" );

   // �⤣�ݭn���Ѽ������A����
   gtk_widget_set_sensitive( GTK_WIDGET( stop_edge   ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( pass_ripple ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( stop_ripple ), FALSE );

   // ��J�w�]��
   gtk_entry_set_text( GTK_ENTRY ( fs ), "10000" );
   gtk_entry_set_text( GTK_ENTRY ( pass_edge ), "480" );

   KIND  = 1;
}


void
on_window_clicked                      (GtkButton       *button,
                                        gpointer         user_data)
{
   GtkWidget *fs          = lookup_widget( GTK_WIDGET( button ),   "fs"        );
   GtkWidget *pass_edge   = lookup_widget( GTK_WIDGET( button ),   "pass_edge" );
   GtkWidget *stop_edge   = lookup_widget( GTK_WIDGET( button ),   "stop_edge" );
   GtkWidget *pass_ripple = lookup_widget( GTK_WIDGET( button ), "pass_ripple" );
   GtkWidget *stop_ripple = lookup_widget( GTK_WIDGET( button ), "stop_ripple" );

   // �⤣�ݭn���Ѽ������A����A�ݭn���Ѽƥ��}�A�P��
   gtk_widget_set_sensitive( GTK_WIDGET( stop_edge   ), TRUE  );
   gtk_widget_set_sensitive( GTK_WIDGET( pass_ripple ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( stop_ripple ), FALSE );

   // ��J�w�]��
   gtk_entry_set_text(GTK_ENTRY ( fs ), "10000" );
   gtk_entry_set_text(GTK_ENTRY ( pass_edge ), "2000" );
   gtk_entry_set_text(GTK_ENTRY ( stop_edge ), "3000" );

   KIND  = 2;
}


void
on_butterworth_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
   GtkWidget *fs          = lookup_widget( GTK_WIDGET( button ),   "fs"        );
   GtkWidget *pass_edge   = lookup_widget( GTK_WIDGET( button ),   "pass_edge" );
   GtkWidget *stop_edge   = lookup_widget( GTK_WIDGET( button ),   "stop_edge" );
   GtkWidget *pass_ripple = lookup_widget( GTK_WIDGET( button ), "pass_ripple" );
   GtkWidget *stop_ripple = lookup_widget( GTK_WIDGET( button ), "stop_ripple" );

   // �⤣�ݭn���Ѽ������A����A�ݭn���Ѽƥ��}�A�P��
   gtk_widget_set_sensitive( GTK_WIDGET( stop_edge   ), TRUE  );
   gtk_widget_set_sensitive( GTK_WIDGET( pass_ripple ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( stop_ripple ), TRUE  );

   // ��J�w�]��
   gtk_entry_set_text( GTK_ENTRY ( fs ), "8000" );
   gtk_entry_set_text( GTK_ENTRY ( pass_edge ), "1200" );
   gtk_entry_set_text( GTK_ENTRY ( stop_edge ), "1500" );
   gtk_entry_set_text( GTK_ENTRY ( stop_ripple ), "25" );

   KIND  = 3;
}


void
on_chebyshev_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
   GtkWidget *fs          = lookup_widget( GTK_WIDGET( button ),   "fs"        );
   GtkWidget *pass_edge   = lookup_widget( GTK_WIDGET( button ),   "pass_edge" );
   GtkWidget *stop_edge   = lookup_widget( GTK_WIDGET( button ),   "stop_edge" );
   GtkWidget *pass_ripple = lookup_widget( GTK_WIDGET( button ), "pass_ripple" );
   GtkWidget *stop_ripple = lookup_widget( GTK_WIDGET( button ), "stop_ripple" );

   // ��ݭn���Ѽƥ��}�A�P��
   gtk_widget_set_sensitive( GTK_WIDGET( stop_edge   ), TRUE  );
   gtk_widget_set_sensitive( GTK_WIDGET( pass_ripple ), TRUE );
   gtk_widget_set_sensitive( GTK_WIDGET( stop_ripple ), TRUE );

   // ��J�w�]��
   gtk_entry_set_text(GTK_ENTRY ( fs ), "20000" );
   gtk_entry_set_text(GTK_ENTRY ( pass_edge ), "5000" );
   gtk_entry_set_text(GTK_ENTRY ( stop_edge ), "7500" );
   gtk_entry_set_text(GTK_ENTRY ( pass_ripple ), "1" );
   gtk_entry_set_text(GTK_ENTRY ( stop_ripple ), "32" );

   KIND  = 4;
}


// toggled ���ƥ�O�|�o�ͦb�P�@�դ����A���ܪ��ɭԡA��p�� A �BB ��� radiobutton �A�ӹw�]�O A ��
// �P��A�p�G�@�� radiobutton B �Q�I�諸�ɭԡA�h radiobutton A ?? toggled �ƥ�|���Q�ҰʡA��
// radiobutton B �� toggle �ƥ�~�|�Q�ҰʡA�]�N�O���A�b�^����A toggle ���N��O���� switch ��
// �F��A?P�@�շ��u�n���󦳵o�͡��}���Ρ������ʧ@�̡A�@�߳��|�Ұʨ� toggled �ƥ�A�o�]�N�O������C�@��
// toggled �ƥ�b�����n�[�J if( gtk_toggle_button_get_active( togglebutton ) ) �H�T�O�ثe��
// �ƥ�O�Q enabled ��?��o���A�Ӥ��O�] disable ��Ĳ�o��
// ���~�A�b�B�� radiobutton ���ɭԡA�O�o�n�� group ���ȩw�q�n�A�P�����k�b�@�_�ΦP�@�� group ID ��
// �Ӧp�G�A�������W�u���o�ˤ@��?? radiobutton ���ɭԡA�h�i���[ group ID ���A�]���t�η|�{�w�O�P�@��
// ���L�i���n�ߺD�A��A�`�O�n���A���n�ѬO�̾a���۰ʤơ��@�~


void
on_fir_input_coefficient_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
   GtkWidget *a_value = lookup_widget( GTK_WIDGET( button ), "a_value" );
   GtkWidget *b_value = lookup_widget( GTK_WIDGET( button ), "b_value" );
   GtkWidget *fs          = lookup_widget( GTK_WIDGET( button ),   "fs"        );

   // ��� optionmenu1 ���ثe�ȡA�H�K�M�w X �B Y �b�Φ�ؤ覡��{
   GtkWidget *optionmenu1 = lookup_widget( GTK_WIDGET( button ), "optionmenu1" );
   GtkWidget *menu = GTK_OPTION_MENU( optionmenu1 ) -> menu;
   GtkWidget *active_item = gtk_menu_get_active( GTK_MENU( menu ) );
   gint mode = g_list_index( GTK_MENU_SHELL( menu ) -> children, active_item );


   // �]�� FIR ���t����{���O�S�� a ���Y�ơA���y�ܻ��A�]�N�O�S���L�h����X
   gtk_widget_set_sensitive( GTK_WIDGET( a_value ), FALSE );

//   gtk_entry_set_text( GTK_ENTRY ( b_value ),
//   "0.09091 0.09091 0.09091 0.09091 0.09091 0.09091 0.09091 0.09091 0.09091 0.09091 0.09091" );
   gtk_entry_set_text( GTK_ENTRY ( b_value ),"4.7 2.2 3.6 2.2 4.7" );

   switch( mode )
   {
      case 3 :
      case 4 :
      {
         gtk_entry_set_text( GTK_ENTRY ( fs ), "10000" );
	 break;
      }
   }
}


void
on_iir_input_coefficient_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
   GtkWidget *a_value = lookup_widget( GTK_WIDGET( button ), "a_value" );
   GtkWidget *b_value = lookup_widget( GTK_WIDGET( button ), "b_value" );
   GtkWidget *fs          = lookup_widget( GTK_WIDGET( button ),   "fs"        );

   // ��� optionmenu1 ���ثe�ȡA�H�K�M�w X �B Y �b�Φ�ؤ覡��{
   GtkWidget *optionmenu1 = lookup_widget( GTK_WIDGET( button ), "optionmenu1" );
   GtkWidget *menu = GTK_OPTION_MENU( optionmenu1 ) -> menu;
   GtkWidget *active_item = gtk_menu_get_active( GTK_MENU( menu ) );
   gint mode = g_list_index( GTK_MENU_SHELL( menu ) -> children, active_item );

   // �]�� FIR �t����{�����S�� a ���Y�ơA���y�ܻ��A�]�N�O�S?��L�h����X
   gtk_widget_set_sensitive( GTK_WIDGET( a_value ), TRUE );

   gtk_entry_set_text( GTK_ENTRY( a_value ), "1 -0.7757" );

   gtk_entry_set_text( GTK_ENTRY( b_value ), "0.1122 0.1122" );

   switch( mode )
   {
      case 3 :
      case 4 :
      {
         gtk_entry_set_text( GTK_ENTRY ( fs ), "25000" );
         break;
      }
   }
}

void
on_print_spectrum_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
   GtkWidget *print_spectrum = lookup_widget( GTK_WIDGET( togglebutton ), "print_spectrum" );
   GtkWidget *printer_name = lookup_widget( GTK_WIDGET( togglebutton ), "printer_name" );

   // �T�w�ثe�O�_�O�n�ϥΦL����ӦC�L�W�й�
   if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( print_spectrum ) ) )
      gtk_widget_set_sensitive( GTK_WIDGET( printer_name ), TRUE );
   else
      gtk_widget_set_sensitive( GTK_WIDGET( printer_name ), FALSE );
}


void
on_clean_data_area_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget *text1, *text2;

  //GtkWidget *appbar1, *togglebutton1;
 // gint yes;
/*

 static gfloat i = 1;

 gfloat j = 500, percent;

      i++;
   percent = i / j;
   //percent = percent * 100;
*/


  text1 = lookup_widget( GTK_WIDGET( button ), "text1");
  text2 = lookup_widget( GTK_WIDGET( button ), "text2");
  gtk_editable_delete_text( GTK_EDITABLE( text1 ), 0, -1);
  gtk_editable_delete_text( GTK_EDITABLE( text2 ), 0, -1);


 // appbar1 = lookup_widget( GTK_WIDGET( button ), "appbar1" );
 // togglebutton1 = lookup_widget( GTK_WIDGET( button ), "togglebutton1" );

  //if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( togglebutton1 ) ) )
    //   gnome_appbar_set_progress( GNOME_APPBAR( appbar1 ), (int)percent );
{


  //      yes = gtk_timeout_add( 1, test, NULL);

}

  //   gnome_appbar_set_progress( GNOME_APPBAR( appbar1 ), percent );
     //gnome_appbar_push( GNOME_APPBAR( appbar1 ),"push 's worked." );
 //    gnome_appbar_set_status( GNOME_APPBAR( appbar1 ),"hello");

 //  gnome_appbar_set_prompt( GNOME_APPBAR( appbar1 ), "prompt", TRUE );
 // else
   /* g_print(" now is %f percentage.\n",
    gtk_progress_get_current_percentage(
    GTK_PROGRESS( gnome_appbar_get_progress( GNOME_APPBAR( appbar1 )) ) ) );
    */

   // gnome_appbar_clear_prompt ( GNOME_APPBAR( appbar1 ) );

  // g_print( " %s \n ", gnome_appbar_get_response( GNOME_APPBAR( appbar1 ) ) );


/*
    g_print(" now is %f percentage.\n",
    gtk_progress_get_percentage_from_value(
    GTK_PROGRESS( gnome_appbar_get_progress( GNOME_APPBAR( appbar1 )) ) ,0.2) );
*/



  //   gnome_appbar_refresh( GNOME_APPBAR( appbar1 ));
   //  gnome_appbar_clear_stack( GNOME_APPBAR( appbar1 ) );
   //  gnome_appbar_pop( GNOME_APPBAR( appbar1 ));

//

}

void
on_inspect_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
   GtkWidget *entry4 = lookup_widget( GTK_WIDGET( button ), "entry4" );

   gchar *file_path = gtk_entry_get_text( GTK_ENTRY( entry4 ) );

   gchar sys_command[ LENGTH ];

   // ���I���h���A�N���|�ϥD�{�������C
   sprintf( sys_command, "gv %s &", file_path );
   
   // system �o�Ө禡�i�H�N�ѼơA���׺ݾ����U�h����
   system( sys_command );

}


