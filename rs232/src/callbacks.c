#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <glib.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

//#include <iostream.h>

// 讀取、寫入 I/O 的時候需要這個函式，有時候偵錯的時候秀在螢幕上也方便嘛！這就叫“所視即所得”，Seeing is believing!
#include <stdio.h>

#include <string.h>

// sys/stat.h 用來使用 stat 這個函式，用他來判別檔案是否存在於既有的位置
#include <sys/stat.h>

// termios.h、fcntl.h 串列埠的協定
#include <termios.h>
#include <fcntl.h>

#include <stdlib.h>

// 用很多數學函式如 sin 、 cos 、 cosh 及常數定義 M_PI = π等等
#include <math.h>

#define MAX_VALUE 1000
#define LENGTH 100

// 用在讀取檔案時，一行之預設長度，用於 fgets 之中，而 fgets 只要求讀到此長度，或是換行即停止
#define SIZE 20

// 用來放置暫存檔案的，如 .gnuplot，及 .data 兩個暫時檔
#define TMP_DIR "/tmp/"

// 串列埠協定的鮑率
gint BAUDRATE;

// 讀取一筆資料之位元數
gint READSIZE;

// 要開啟的序列埠，預設是 ttyS0，也就是 com1
gchar *PORT;

// 就是濾波器的型號 FIR
//                        1 ： 移動平均濾波器( Moving average filter )
//                        2 ： 視窗濾波器( Window filter )：
//                             漢尼視窗 44 、漢明視窗 55 、黑人視窗 75
//                                                          p.s.視窗旁之數字為其止帶衰減率
//               IIR
//                        3 ： 油奶值得濾波器( Butterworth filter )
//                        4 ： 契比雪夫濾波器Ⅰ型( Chebyshev filter )
//
//
// 用假設的轉移函數代入 a 、b 值，若為 IIR 則有 a 值，若為 FIR 則僅有 b 值而已，在此令此
//                        5 ： 假設之轉移函數之 IIR 或 FIR 之濾波器
gint KIND;

// 可以說是解析度，也就是值越大，圖形的線條越平滑，但是一般來說，其實 100 就已經相當夠用了，再大也只是浪費時間而已
gint pixel;

// 開啟 tmp_data_name 時所用的檔案指標，設成全域變數是因為每個濾波器的函式都需要這個檔案指標
FILE *tmp_data;

// 頻譜圖中的總值，若是平滑度取的不一樣，則值會改變喔！
gfloat total = 0;

// 頻譜圖中的最大值
gfloat max = 0;

// 宣告為全域變數，讓大家是“有福同享”嘛，相對的，如果我的程式有問題的話，會變成“有難同當”了啊！
GtkWidget *statusbar;

// 製作一個全域的 widget 指標，好讓所有的函式可以找到 widget 指標，
// 在這裡是“特別”用在 file_selection_dialog
GtkButton *some_button;

// 宣告一個全域的 widget 令其檔名為 file_selection_box ，而在
// on_save_as_numeral_clicked 事件被觸發的時候可以產生一個圖形界面，好讓 store_input_file 可以
// 找到 selection dialog
GtkWidget *file_selection_box;



void test( gint num );



// on_rs232_show 是代表 rs232 這個程式在一開始的時候就執行的函式，也就是只有執行
// 這一次，也就是說，當程式在讀取 main.c 中，執行到 gtk_widget_show (rs232)
// 這個函式，立刻執行 on_rs232_show 這個函式。正因為只執行這一次，所以我們可以把
// 整個系統的初設值統統設定在此處。
void
on_rs232_show                          (GtkWidget       *widget,
                                        gpointer         user_data)
{
   // 要用假設的轉移函數係數輸入的元件
   GtkWidget *fir_input_coefficient = lookup_widget( GTK_WIDGET( widget ), "fir_input_coefficient" );
   GtkWidget *iir_input_coefficient = lookup_widget( GTK_WIDGET( widget ), "iir_input_coefficient" );
   GtkWidget *a_value = lookup_widget( GTK_WIDGET( widget ), "a_value" );
   GtkWidget *b_value = lookup_widget( GTK_WIDGET( widget ), "b_value" );

   // 一開始假設還不需要列印任何東西
   GtkWidget *printer_name = lookup_widget( GTK_WIDGET( widget ), "printer_name" );

   // 希望視窗一打開的時候預設是以輸入明細規格的濾波器
   // 假設的轉移函數係數輸入的元件，全部失能
   gtk_widget_set_sensitive( GTK_WIDGET( fir_input_coefficient ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( iir_input_coefficient ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( a_value ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( b_value ), FALSE );

   gtk_widget_set_sensitive( GTK_WIDGET( printer_name ), FALSE );

   statusbar = lookup_widget( GTK_WIDGET( widget ), "statusbar1");

   // 預設值 4800 的鮑率
   BAUDRATE = B4800;

   // 讀取一筆資料其位元數
   READSIZE  = 16;

   // 要開啟的序列埠，預設是 ttyS0，也就是 com1
   PORT = "/dev/ttyS0";

   // 預設的濾波器種類，正所謂用當然是用最好的囉，在此是契比雪夫濾波器
   //KIND = 4;
   KIND = 6;
}

void
on_swap_clicked                      (GtkButton       *button,
                                        gpointer         user_data)
{
   // 存放原始資料如：US,NT,+  14.14 這樣的型式
   gchar buf[ READSIZE + 1 ];

   // extract 不是口香糖的品牌( 無糖口香糖 )，而是由 buf 所得到的純數值資料
   // 如果 buf = US,NT,+  14.14 這樣的話，則 extract = +  14.14
   gchar *extract;

   // 判別是否 toggle_button 是否沈下去，若是則為 true
   gboolean enforce_write;

   // 將是一個要被寫入資料檔的檔名指標
   FILE *savefile = NULL;

   // oldtio 是準備用來放置原來的串列埠的設定，newtio 是用來放置要用來打開串列埠的設定
   struct termios oldtio;
   struct termios newtio;

   // 用來測試目前所希望的路徑之檔名，是否早已存在，
   struct stat examine;

   // 用來放置指向序列埠的指標
   gint fd;

   // 用來放要貯存資料的路徑檔名
   gchar *path;

   gint i = 0, num;

   // 由秤盤所傳出未經處理的原始資料值，放置在 text 中秀出來
   GtkWidget *protodata = lookup_widget(GTK_WIDGET(button), "text1");

   // text 在此到時候是放置已處理過的數值資料
   GtkWidget *numeral = lookup_widget(GTK_WIDGET(button), "text2");

   // 這個 button 跟 checkbutton 有一樣的用法，比較不一樣的是，他可以把文字直接印在
   // 按鈕，正因為如此，所以他的 button 也比較大，是可以改變大小的， 這個變數在此的意思
   // 是說當按下之後，則於開啟寫入檔案時，不管那個檔案先前是否存在，都一定強迫寫入，
   GtkWidget *canwrite = lookup_widget( GTK_WIDGET( button ), "togglebutton1");

   // 找尋用來放置準備有幾筆資料要讀取的 entry
   GtkWidget *amount = lookup_widget( GTK_WIDGET( button ), "entry2");

   // 找尋用來放置其硬體資料位元組長度的 entry
   GtkWidget *bytes = lookup_widget( GTK_WIDGET( button ), "entry5" );

   // 找尋要寫入的資料檔路徑及其檔案的 entry
   GtkWidget *saveaspath = lookup_widget( GTK_WIDGET( button ), "entry1" );

   // 因為在接收資料的時候，會很久，所以特地做一個進度棒，來表示目前的情形
//   GtkWidget *progressbar1 = lookup_widget( GTK_WIDGET( button ), "progressbar1" );

   // path 放著要寫入的資料檔路徑及其檔案
   path = gtk_entry_get_text( GTK_ENTRY( saveaspath ) );

   // 因為 gtk_entry_get_text 所得到的回傳值是一個字串值，所以用 atoi 轉換為數字
   num = atoi( gtk_entry_get_text( GTK_ENTRY( amount ) ) );

   // 因為 gtk_entry_get_text 所得到的回傳值是一個字串值，所以用 atoi 轉換為數字
   READSIZE = atoi( gtk_entry_get_text( GTK_ENTRY( bytes ) ) );

   // 辨別目前的的 toggle_button 是沈下去的，還是浮起來，作為是否要強迫寫入檔案的依據
   enforce_write = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( canwrite ) );


   // 當 enforce_write 為 true 時，或是指定的檔案不存在時則可寫入
   // stat 的錯誤訊息可以參考 /usr/include/asm/errno.h 這個檔案
   if( ( stat( path, &examine ) == ( 0 ) ) && ( !enforce_write ) )
      // 當檔案早已存在系統之中時，所給的狀態列提示
      gtk_statusbar_push( GTK_STATUSBAR( statusbar ), 1
      , "指定的檔案早已存在檔案系統中，請選取其他的檔案，或是按下“強制寫入檔案”，強迫寫入檔案。" );

      // 當開啟路徑檔案 path 失敗的時候，在狀態列上顯示出警告信號，
      // 縱使失敗也繼續執行，所以沒有加入 return，也就是說，即使不存入檔案也可以傳入資料
   else if( ( savefile = fopen( path , "w" ) ) == NULL )
      gtk_statusbar_push( GTK_STATUSBAR( statusbar ), 1
      , "你指定的檔案名稱無法建立，請再確認一次。" );


   // PORT 為欲開啟的序列裝置其預設是 com1 也就是 /dev/ttyS0，O_NOCTTY
   // 這個參數是希望這個行程“不要”被當作是控制終端的模式( controlling terminal )
   // ，換句話說，也就是並不會因為你按了 Ctrl + C 然後就能夠中止程式的進行，
   // 而 O_RDWR 這個參數則是讓裝置可被“讀”又可被“寫”入
   fd = open( PORT, O_RDWR | O_NOCTTY );

   // 當開啟裝置失敗時，fd < 0 ，然後在狀態列上面秀出訊息警告
   // 當 fd > 0 的時候，則開始進入初設序列埠的階段
   if ( fd < 0 )
   {
      gtk_statusbar_push( GTK_STATUSBAR( statusbar ), 1
      , "指定的裝置開啟失敗，請自行查明原因。" );
      if ( savefile != NULL )
         fclose( savefile );
      return;
   }
   else
   {  // 奇怪的是，在秀在“訊息列”的字當中若是加入“功”這個字， compile 就會出問題
      gtk_statusbar_push( GTK_STATUSBAR( statusbar ), 1
      , "指定的裝置已開啟。" );

      // 把先前的序?C埠的設定貯存起來
      tcgetattr( fd, &oldtio );

      // bzero 會將 newtio 所指的記憶體區域前 sizeof( newtio ) 個位元組，
      // 全部設為零值，相當於呼叫 memset( void *s ), 0,size_t n );
      bzero( &newtio, sizeof( newtio ) );

      // 在 /usr/include/bits/termios.h 之中有 termios 的結構體，如：
      // c_cflag、c_iflag、c_oflag、c_lflag、c_cc 的旗標設定值可在此找到

      // c_cflag 這個旗標控制著鮑率、位元數、同位元、停止位元以及硬體流量控制
      // 其最後結果由“|” bitwise operator 逐位運算元所得到?熊痕G來決定
      // BAUDRATE 設定傳輸的速率，
      // CRTSCTS 輸出流量的硬體流控制
      // CS8 為 8n1，8 位元檢查，一個終止位元( character stop 8 )
      // CLOCAL 本地連線，不具數據?鰼惆謋\能
      // CREAD 使致能接收字元
      newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;

      // c_iflag 這個旗標決定如何處理接收到的字元，若不加處理則令為 0即?? raw模式
      // 其最後結果由“|”bitwise operator 逐位運算元所得到的結果來決定
      // IGNPAR 經同位元檢查後，不要管錯誤的發生
      // ICRNL 將 CR 對應成 NL ，否則當輸入訊號有 CR 時不會終止輸入?A
      // 再不然如果沒有其他的輸入處理的情形下，也可以把裝置設定成 raw 模式，
      newtio.c_iflag = IGNPAR | ICRNL;

      // c_iflag 決定了輸出模式，和 c_iflag 一樣，raw 原始的模式輸出則令為 0
      newtio.c_oflag = 0;

      // c_lflag 這個旗標是一個決定如何處理由序列裝置所得到的字元，一般來說通常
      // 都設定為 canonical 或是 raw 的模式，?]就是給他 ICANON 或是 0 其中一個值
      // ICANON 為致能標準輸入，使所有回應機制停用，並不送出信號以便於呼叫程式
      newtio.c_lflag = 0;

      // c_cc 這個陣列包含了一些像是 timeout 的參數的字義

      // VINTR 所代表的陣值就是中斷( interrupt )，就是 Ctrl + C
      newtio.c_cc[ VINTR ] = 0;

      // VQUIT 所代表的陣值就是放棄( quit )，就是 Ctrl + Z
      newtio.c_cc[ VQUIT ] = 0;

      // VERASE 所代表的陣值就是清除( erase )，就是 backspace
      newtio.c_cc[ VERASE ] = 0;

      // VKILL 所代表的陣值就是清除一行( kill-line )，也就是 Ctrl + U
      newtio.c_cc[ VKILL ] = 0;

      // VEOF 所代表的陣值就是到最後一行( end of file )，也就是 Ctrl + D
      newtio.c_cc[ VEOF ] = 4;

      // 不使用分割字元組的計時器
      newtio.c_cc[ VTIME ] = 0;

      // VMIN 所代表的陣值表示是要讀取的最小位元數目
      newtio.c_cc[ VMIN ] = READSIZE;

      // '\0'
      newtio.c_cc[ VSWTC ] = 0;

      // Ctrl + Q
      newtio.c_cc[ VSTART ] = 0;

      // Ctrl + S
      newtio.c_cc[ VSTOP ] = 0;

      // '\0'，也就是 Carriage return ( CR )，也就是 Enter
      newtio.c_cc[ VEOL ] = 0;

      // Ctrl + R
      newtio.c_cc[ VREPRINT ] = 0;

      // Ctrl + u
      newtio.c_cc[ VDISCARD ] = 0;

      // Ctrl + w
      newtio.c_cc[ VWERASE ] = 0;

      // second end-of-line 就是 LF
      newtio.c_cc[ VEOL2 ] = 0;

      // 清除設定
      tcflush( fd, TCIFLUSH );

      // 將 newtio 的設定，且 TCSANOW 命令其設定立即生效
      tcsetattr( fd, TCSANOW, &newtio );

      // 前一筆資料，不做處理，避免因為資料不完整的情形
      bzero( buf, sizeof( buf ) );
      read( fd, buf, READSIZE );

      while( ( i++ ) != num )
      {
         // 把 buf 這塊記憶體清除乾淨
         bzero( buf, sizeof( buf ) );

	 // 讀取由 fd 指向的檔案裝置的資料，其實也就是檔案指標是指到/dev/ttyS
	 // ，然後將其位元數目為 READSIZE 的位元組，送到所指的記憶體，也就是 buf
	 // read 的傳回值為實際讀取到的位元組數，如果傳回 0 ，表示已到達檔案結尾
	 // 或者是無可讀取的資料，此外檔案讀寫位置會隨讀取到的位元組移動。
	 // 判斷是否所傳回的位元組是否大小一致，確定沒有讀錯，只要有錯，此次迴圈就不要做。
	 if( read( fd, buf, READSIZE ) != READSIZE )
            continue;

	 // strpbrk 這個函式是用來找出引數 buf 字串中，最先出現存在引數“+-”的字
         // 因為原始的未處理資料為 US,NT,+  14.14 這樣的型式，在此
	 // buf = US,NT,+  14.14 ，而經過函式處理後，extract = +  14.14 這樣的值
//	 extract = strpbrk( buf, "+-");

	 // 在此若 buf = US,NT,+  14.14 ，則 extract = 14.14，也就是不帶正負號
	 extract = strpbrk( buf, "0123456789" );

         // 把 buf 的資料插入 protodata 中，而 extract 的資料插入
         // numeral 之中 gtk_text_insert 這個函式，第一個參數為指定的 text ，
	 // 第一個 NULL 的參數為使用的字型，NULL 表示用目前的字型
	 // 第二個 NULL 的參數為這個 text 裡的字體顏色， NULL 為用目前的顏色
         // 第三個 NULL 的參數為這個 text 裡的背景顏色， NULL 為用目前的顏色
	 // 第四個 buf 的參數為要插入的字串，
	 // 第五個參數位置則為 buf 的大小，若給 -1 值，則表示全部秀出來
	 gtk_text_insert( GTK_TEXT( protodata ), NULL, NULL, NULL, buf, -1 );
         gtk_text_insert( GTK_TEXT( numeral ), NULL, NULL, NULL, extract, strlen( extract ) );

     //    gtk_timeout_add( 1., test, num );


         // 確定先前檔案是否有開啟成功，如果有則寫入檔案之中
	 if ( savefile != NULL )
	 {
	    fprintf( savefile, "%s", extract );
	    fflush( savefile );
	 }

      }

      // 當迴圈結束時，也就是 num 筆資料接完的時候，此時，把 oldtio 的設定
      // “立即”寫回 fd 所指的裝置檔案中
      tcsetattr( fd, TCSANOW, &oldtio );

      // 確定先前檔案是否有開啟成功，如果有則關閉檔案
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

   // /usr/include/bits/termios.h 有著其定義值
   // 一旦使用者有著想要改變鮑率的意圖，則 changed 的事件會立即觸發， BAUDRATE 便會改變
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

   // 下面這個秀出來可以看到 BAUDRATE 在 /usr/include/bits/termios.h 的定義值
   // 但是老實說，我覺得怪怪的，雖然程式是執行正常的。
   // g_print(" %d\n ", BAUDRATE );
}



void
on_combo_entry_port_changed            (GtkEditable     *editable,
                                        gpointer         user_data)
{
   GtkWidget *port = lookup_widget( GTK_WIDGET( editable ), "combo_entry_port" );

   // 一旦使用者有著想要改變序列埠的意圖，則 changed 的事件會立即觸發， PORT 便會改變
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


// 把從 file_selection_box 決定後的檔名放到 entry1 之中
// ，如果取消的話，當然是不要放入囉！
void store_input_file( GtkFileSelection *file_selection, gpointer user_data)
{
   // 從 file_selection_box 得來的 path ，放入於此處
   gchar *in_filename;

   GtkWidget *file_path;

   // 判斷是要決定 eps 的路徑，還是 numeral 的
   if( user_data == "eps" )
      // 表示是 eps 路徑的要求
      file_path = lookup_widget( GTK_WIDGET( some_button ), "entry4" );
   else
      // 表示是 numeral 路徑的要求
      file_path = lookup_widget( GTK_WIDGET( some_button ), "entry1" );

   // 從 file_selection_box 之中得到檔名及其路徑。
   in_filename = gtk_file_selection_get_filename
      ( GTK_FILE_SELECTION( file_selection_box ) );

   // 把檔名及其路徑名丟到 entry1 裡面，也就是 file_path 啦！
   gtk_entry_set_text( GTK_ENTRY( file_path ), in_filename );
}

void
on_save_as_numeral_clicked                      (GtkButton       *button,
                                        gpointer         user_data)
{
   // 指定 button 的指標指給 some_button ，讓 store_input_file
   // 這個函式可?H找到需要的widget
   some_button = button;

   // 創造一個 file_selection_box，令其標題為 Choose the data file.....
   file_selection_box = gtk_file_selection_new( "請自行加上副檔名，如資料檔(txt)或圖檔(eps、pdf、ps……)" );

   // 因為標題太長了，所?H我把這個檔案選擇這個視窗改變了一下大小
   gtk_widget_set_usize (file_selection_box, 700, -2);

   // 把 file_selection_box裡面的 "ok" 按鈕作事件觸發到 store_input_file 的函式
   // 各位看倌可否注意到 gtk_signal_connect 和 gtk_signal_connect_object 的不同嗎？
   // 原則上這兩個函式是一樣的，唯一不同的地方是 gtk_signal_connect_object 可以傳
   // widget ，也就是說事實上， gtk_widget_destroy 這個事件，是需要一個 widge 的參數
   // 不然他怎麼知道要摧毀什麼
   gtk_signal_connect( GTK_OBJECT
      ( GTK_FILE_SELECTION( file_selection_box ) -> ok_button ),
      "clicked", GTK_SIGNAL_FUNC( store_input_file ), user_data );

   // 在?鬗U“ok”這個按鈕之後把 file_selection_box 這個 widget
   // 毀掉，就是消失在螢幕上啦！
   gtk_signal_connect_object( GTK_OBJECT
      ( GTK_FILE_SELECTION( file_selection_box ) -> ok_button ),
      "clicked", GTK_SIGNAL_FUNC( gtk_widget_destroy ),
      ( gpointer )file_selection_box );

   // 在按下“cancel”這個按鈕之後把 file_selection_box
   // 這個 widget 毀掉，就是消失在螢幕上啦！而沒有作 store_input_file 的動作
   gtk_signal_connect_object( GTK_OBJECT
      ( GTK_FILE_SELECTION( file_selection_box ) -> cancel_button ),
      "clicked", GTK_SIGNAL_FUNC( gtk_widget_destroy ),
      ( gpointer )file_selection_box );

   // 秀出 file_selection_box 這個視窗出來！
   gtk_widget_show( file_selection_box );
}

// on_rs232_delete_event 這個事件會發生在當你點選了視窗右上角的“╳”，
// 才會發生，也就是所謂?? window manager
gboolean
on_rs232_delete_event                  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{

   // gtk_main_quit() 這個函式是讓主視窗直接摧毀的功能，而難道其他的小 widget
   // 不需要去摧毀嗎？相信你有這樣的疑問？請看 main.c 中的 gtk_widget_show (rs232);
   // 在 GTK+ 程式設計的觀念?矰丑A容器 container 的觀念是十分重要的，其實整個
   // GtkWindow 就是一個“最”大的容器，其他還有諸如 Horizontal box 還是
   // Vertical box 以及 Table 也是容器的一種，不過他們都是包含在 GtkWindow 這
   // 個最大的容器裡面。所以一旦用了 gtk_main_quit() 就等於把所有包含在 GtkWindow
   // 底下的元件統統終止掉！
   //gtk_main_quit();

   // 終止所有程式，並且傳回 exit code 給呼叫者，並且釋放所有源自於 GTK 的資源

   gtk_exit( 0 );


   // 理論上下面這一行不會執行到，但是是為了編譯能夠通過
   // 一旦傳回值為 TRUE 則系統就會防範關閉的動作，而 FALSE 是使系統發出 destroy 的訊號
   return FALSE;
}

void
on_spectrum_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
   // 抓取 optionmenu1 的目前值，以便決定 X 、 Y 軸用何種方式表現
   GtkWidget *optionmenu1 = lookup_widget( GTK_WIDGET( button ), "optionmenu1" );
   GtkWidget *menu = GTK_OPTION_MENU( optionmenu1 ) -> menu;
   GtkWidget *active_item = gtk_menu_get_active( GTK_MENU( menu ) );
   gint mode = g_list_index( GTK_MENU_SHELL( menu ) -> children, active_item );


   // 此值決定要不要將頻譜圖用印表機繪出
   GtkWidget *print_spectrum = lookup_widget( GTK_WIDGET( button ), "print_spectrum" );

   // 此值決定要不要用大張的頻譜圖
   GtkWidget *big_spectrum = lookup_widget( GTK_WIDGET( button ), "big_spectrum" );

   // 此 widget 來抓視窗上取樣頻率的值，然後秀在圖檔的右上角，因為這個值最重要，所以秀在那
   GtkWidget *fs_value = lookup_widget( GTK_WIDGET( button ), "fs" );

   //
   GtkWidget *fir_input_coefficient = lookup_widget( GTK_WIDGET( button ), "fir_input_coefficient" );

   // 就是 eps 的輸出路徑
   GtkWidget *saveaspath = lookup_widget( GTK_WIDGET( button ), "entry4" );

   // 就是 X 軸的解析度
   GtkWidget *point = lookup_widget( GTK_WIDGET( button ), "point" );

   // 印表機的電腦名稱
   GtkWidget *printer_name = lookup_widget( GTK_WIDGET( button ), "printer_name" );

   // 決定是否使用印表機將頻譜印出
   gboolean enable_printer;

   // --------------------------------------------------------------
   // tmp_script_name 是用來放 gnuplot 要讀取的 script 命令
   // tmp_data_name 是用來放 data 也就是頻譜圖的 X、Y 值檔案的檔案
   // tmp_data_name 將要被 gnuplot 讀入的資料檔案，也就是裡面的資料將有兩行，
   // 第一行是在頻譜上 X 的值，第二行是在頻譜上 Y 的值，兩個值成一個點，然會劃成圖形
   // 輸出在 output_file_name 之中
   gchar tmp_script_name[ LENGTH ], tmp_data_name[ LENGTH ];

   // 開啟 tmp_script_name 時所用的檔案指標;
   FILE *tmp_script;

   gchar sys_command[ LENGTH ];

   // 要輸出的 eps 的檔案名稱
   gchar *output_file_name;

   // 在 eps 檔之中所用到的參數，標題， X 座標以及 Y 座標的名稱，xlabel 還需要其他一些訊息
   // title 在 switch 的選項中如果沒有先給一個初設值，則會有警告，
   gchar xlabel[ LENGTH ] = "", *ylabel = "", temp[ LENGTH ] = "", *title = "";

   // 完成頻譜後，在訊息列所顯示的資訊
   gchar *spectrum_message = "";

   // 放在頻譜的右上角的
   gchar banner[ LENGTH ] = "fs = ";


 /*
   // b 值就是轉移函數的係數值，因為是 FIR 濾波器，所以只有 b 值，沒有 a 值
   gfloat *b = 0;

   gint i = 0;
*/
   // -------------------------------------------------------------

   // 決定是否使用印表機印出結果
   enable_printer = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( print_spectrum ) );

   // 決定 X 軸的解析度如何
   pixel = atoi( gtk_entry_get_text( GTK_ENTRY( point ) ) );

   system("clear");

   // 用 getpid() 這個函式隨機產生一個在 /tmp/ 底下且副檔名為 gnuplot 的script檔，
   // 到時候可供 gnuplot 去讀取。
   sprintf( tmp_script_name, "%s%d", TMP_DIR, getpid( ) );
   strcat( tmp_script_name, ".gnuplot");

   // 用 getpid() 這個函式隨機產生一個在 /tmp 底下且副檔名為 data 的資料檔，
   // 可供給 gnuplot 資料並且來劃出圖形
   sprintf( tmp_data_name, "%s%d", TMP_DIR, getpid( ) );
   strcat( tmp_data_name, ".data");

   // 打開 script 和 data ，只要其中一個打不開就放棄。
   if( ( ( tmp_script = fopen( tmp_script_name, "w" )) == NULL ) ||
      ( ( tmp_data   = fopen( tmp_data_name,   "w" )) == NULL ) )
   {
      gtk_statusbar_push( GTK_STATUSBAR( statusbar ), 1
	 , "在產生暫時的 script 還是 data 檔案的時候，發生無法開啟的情形。" );
      return;
   }

   // 設定要輸出的 eps 檔案，及其位置
   output_file_name = gtk_entry_get_text( GTK_ENTRY( saveaspath ) );

   // 選擇是用哪?@種濾波器
   switch( KIND )
   {
      case 1 :
      {
         FIR_Moving_Average( button, mode );
	 spectrum_message =
	 "移動平移濾波器的頻譜圖檔已經劃好了，其結果就放在其“儲存路徑”中";
	 break;
      }
      case 2 :
      {
         FIR_Window( button, mode );
	 spectrum_message =
	 "視窗濾波器的頻譜圖檔已經劃好了，其結果就放在其“儲存路徑”中";
	 break;
      }
      case 3 :
      {
         IIR_Butterworth( button, mode );
         spectrum_message =
	 "奶油值得濾波器的頻譜圖檔已經劃好了，其結果就放在其“儲存路徑”中";
	 break;
      }
      case 4 :
      {
         IIR_Chebyshev( button, mode );
	 spectrum_message =
	 "契比雪夫濾波器的頻譜圖檔已經劃好了，其結果就放在其“儲存路徑”中";
	 break;
      }
      case 5 :
      {
         Transfer_function( button, mode );
	 spectrum_message =
	 "由假設的係數所完成的頻譜圖檔已經劃好了，其結果就放在其“儲存路徑”中";
	 break;
      }
      case 6:
      {
         protodata( button, mode );
	 spectrum_message =
	 "由假設的係數所完成的頻譜圖檔已經劃好了，其結果就放在其“儲存路徑”中";
	 break;
       }
   }


   // 把 max 的值加進去
   strcat( xlabel,"max = " );
   sprintf( temp, "%f,", max );
   strcat( xlabel, temp );
   max = 0;

   // 把 total 的值加進去
   strcat( xlabel," total = " );
   sprintf( temp, "%f", total );
   strcat( xlabel, temp );
   total = 0;

   // X Y 軸的可能組合會有四種情況
   switch( mode )
   {
      case 0 :
      case 1 :
      {
         // 以數位頻率做為 X 軸表示方式，並且把單位靠到偏右邊去，才不會跟 total 的值有所混亂
         strcat( xlabel,"           omega( PI )" );

	 // 數位頻率下的一般增益
         ylabel = "| H( omega ) |";

	 break;
      }
      case 2 :
      {
         // 以數位頻率做為 X 軸表示方式，並且把單位靠到偏右邊去，才不會跟 total 的值有所混亂
         strcat( xlabel,"           omega( PI )" );

	 // 類比頻率下的音量增益
         ylabel = "| H( f ) |( decibel )";

	 break;
      }
      case 3 :
      {

         // 以類比頻率做為 X 軸表示方式，並且把單位靠到偏右邊去，才不會跟 total 的值有所混亂
         strcat( xlabel,"                    f( HZ )" );

	 // 數位頻率下的一般增益
         ylabel = "| H( omega ) |";
	 break;
      }
      case 4 :
      {
         // 以類比頻率做為 X 軸表示方式，並且把單位靠到偏右邊去，才不會跟 total 的值有所混亂
         strcat( xlabel,"                    f( HZ )" );

	 // 類比頻率下的音量增益
         ylabel = "| H( f ) |( decibel )";
	 break;
      }
   }

   switch( KIND )
   {
      // 因為各種濾波器產生不同的標題
      case 1 : title = "FIR Moving Average filter"; break;
      case 2 : title = "FIR Window filter";         break;
      case 3 : title = "IIR Butterworth filter";    break;
      case 4 : title = "IIR Chebyshev filter";      break;
      case 5 :
      {
         if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( fir_input_coefficient ) ) )
            // 當轉移函數是求 FIR 的時候
	    title = "FIR filter derives directly from transfer function";
         else
	    // 當轉移函數是求 IIR 的時候
	    title = "IIR filter derives directly from transfer function";

	                                           break;
      }
      case 6 : title = "protodata filter";          break;
   }

   // 把 banner 變成 Frequency Sampling = 多少頻率的型式
   strcat( banner, gtk_entry_get_text( GTK_ENTRY( fs_value ) ) );
   strcat( banner, " HZ" );

   // 把命令寫到 tmp_script_name 這個檔名裡面去，
   // set terminal postscript 後面可接的參數可以有landscape, portrait,
   // eps 以及 default 四種情形，用 eps 的時候，當然就是希望 gnuplot 劃出的圖形
   // 可?H輸出成一個 eps 的檔案，不過如果你什麼都不加也可以，這時候劃出來的圖形比較
   // 大張，加了 eps 這個參數之後，反而小張，此外，如果你要輸出成 其他的檔案如同
   // ps 檔、或者是 pdf 檔，也都可以直接在儲存路徑上加上所需要的檔名即可輸出成你
   // 所需要的格式
   // color 用了之後，你就會知道什麼叫做“人生是彩色的”，不用的話，你的“人生就是黑白的”
   if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( big_spectrum ) ) )
      fprintf( tmp_script, "set term postscript color" );
   else
      fprintf( tmp_script, "set term postscript eps color" );

   fflush( tmp_script );

   // 這一段就真的不太清楚了
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

   // 把圖形劃出並且把各點連在一起，且各點皆顯示，而 t 可以在右上角顯示此濾波器之取樣頻率，
   // linespoints 是有“線”又有“點”
   // 加了 smooth csplines 是可以把線條平滑化處理
   // fprintf( tmp_script, "plot '%s' using 1:2 smooth csplines t '%s' with lines\n", tmp_data_name , banner );

   fprintf( tmp_script, "plot '%s' using 1:2 t '%s' with lines\n", tmp_data_name , banner );
   fflush( tmp_script );

   // 關閉 tmp_script
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
	 , "當在執行 gnuplot 的時候，有錯誤發生，請自行解決 gnuplot 的問題後，再執行頻譜的繪製" );
   else
      gtk_statusbar_push( GTK_STATUSBAR( statusbar ), 1
	 , spectrum_message );

   // 執行結果後，立刻把 script 以及把 data 殺掉
   sprintf( sys_command, "rm -f %s %s", tmp_script_name, tmp_data_name );

   // system 這個函式可以將參數，丟到終端機底下去執行
   system( sys_command );
}

// KIND = 1

void
FIR_Moving_Average( GtkButton *button, gint mode )
{
   // 這些 widget 是這個函式所需要的
   GtkWidget *frequency_sampling = lookup_widget( GTK_WIDGET( button ), "fs" );
   GtkWidget *pass_band_cutoff = lookup_widget( GTK_WIDGET( button ), "pass_edge" );

   //   以下是實現一個FIR( finite impulse response) 的濾波器，產生其頻譜圖其做法
   //   是完成一個 Moving average 的低通濾波器，再此我們先行假設其階數為“11”階，
   //   此外，以下的程式中，有一些地方都是應用到：Ωfs = 2πf也就是


   //                 數位頻率 × 取樣頻率 ＝ 2 π × 類比頻率

   //      通常來說，一個 FIR 濾波器的設計，其 window 在選用的時候，應該都是根據其
   //   止帶衰減率（stop band attenuation）而選擇的時候，是選擇最接近的止帶衰減率
   //   如果說需求的值僅大於某一視窗的值些許，則必須選擇更大止帶衰減率的視窗，比如說
   //   ，需求要是46 DB，很明顯地，是很接近 Hanning 的45 DB，可是為了要達成有46 DB
   //   的需求則必須要用 Hamming 的視窗，有但是，而其 window 的表格在各大 DSP
   //   數位信號處理的?挭y應該都有所有介紹一些。

   //   以下各個參數是虛擬的。
   //    interval 純粹是一個圖上的各點在X座標上的間距

   gint i = 0 ,j = 0;

   // 通帶截止頻率
   //gfloat cut_off_edge = 480;

   gfloat cut_off_edge = atof( gtk_entry_get_text( GTK_ENTRY( pass_band_cutoff ) ) );


   // 取樣頻率
   //gfloat fs = 10000;

   gfloat fs = atof( gtk_entry_get_text( GTK_ENTRY( frequency_sampling ) ) );

   //   floor 這個函數是取整數值，且是無條件捨去小數點後面的數字，其代表意義??
   //   在頻譜X座標上面所要取的數目。由於內定假設其止帶衰減率為40 DB ，
   //   故由FIR的視窗表，得知應該使用 Hanning 視窗，其 N 的算法為

   //		    N = 3.32 * 取樣頻率 / 轉換頻帶


   // 數位頻率 Ω 於 cut_off_edge 的求法， M_PI 是 π ，在 math.h 之中有定義
   gfloat omega_cut_off = 2 * M_PI * cut_off_edge / fs;

   gint N = ceil( M_PI / omega_cut_off );

   // 在頻譜圖上最大的頻率值，此值相當於 Ω = π
   gfloat f_max = fs / 2, omega_max = M_PI ;

   // 這個是代表從 0 ～ f_max，或者說是 0 ～ Ω之間其所間隔的頻率，
   // 亦或是想成是在頻率 X 軸上面的間隔

   gfloat f_interval = f_max / pixel, omega_interval = omega_max / pixel;

   gfloat f = 0, omega = 0;


   //   由尤拉式 Euler's identity 可知，其對數的乘冪若為複數（ complex number ）
   //   的話則可使對數型式成為

   //                     cos(θ) - jsin(θ)

   //   而下面的變數宣告之所以要分開是因為 math.h 的函數中沒有處理“對數，且其乘冪為
   //   複數的型式”，在此特別分為實部和虛部，因為在計算其增益大小（ Gain ）的時候，
   //   只是單純把實部（real part），以及虛部（imaginary）兩個部份作完全平方根的處
   //   理，至於若是要研究相位( phase )的部分的話，則要考慮有沒有負數的問題！

   gfloat H_real = 0, H_imaginary = 0;


   //   此兩個變數，為實部，以及虛部的總和是準備用來做開根號平方和，其結果便為增益Gain
   //   |H(Ω)|，也就是我們的 Y 軸啦！

   gfloat sum_real = 0, sum_imaginary = 0;


   //   gain 和 sum_real 以及 sum_imaginary 有著這樣的關係：

   //                  gain = √( sum_real平方 + sum_imaginary平方 )

   //   gain就是|H(Ω)|，然而 gain_in_db 和 gain 是有著這樣的關係：

   //                  gain_in_db = 20*log( gain )

   //   其實就是聲音的大小，db 是聲音的單位，叫音貝 ( decibels )

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
	       // 表示使用數位頻率表示 X 軸
               H_real = cos( j * omega );
               H_imaginary = sin( j * omega );
	       break;
	    }

	    case 3 :
	    case 4 :
	    {
               // 表示使用類比頻率表示 X 軸
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
            // 表示使用數位頻率表示 X 軸，一般增益表示 Y 軸
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI , gain );

	    // X 軸為數位頻率時的累加
	    omega= omega + omega_interval;

	    break;
	 }
	 case 2 :
	 {
            // 表示使用數位頻率表示 X 軸，音量增益表示 Y 軸
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI, gain_in_db );

	    // X 軸為數位頻率時的累加
	    omega= omega + omega_interval;

	    break;
	 }
	 case 3 :
	 {
	    // 表示使用類比頻率表示 X 軸，一般增益表示 Y 軸
            fprintf( tmp_data, "%f \t %f\n", f, gain );

	    // X 軸為類比頻率時的累加
	    f = f + f_interval;

	    break;
	 }
	 case 4 :
	 {
            // 表示使用類比頻率表示 X 軸，音量增益表示 Y 軸
	    fprintf( tmp_data, "%f \t %f\n", f, gain_in_db );

	    // X 軸為類比頻率時的累加
	    f = f + f_interval;

	    break;
	 }
      }

      fflush( tmp_data );
      sum_real = 0;
      sum_imaginary = 0;
   }

   // 先行關閉 tmp_data 好讓 gnuplot 等一下可以讀這個暫存的資料檔
   fclose( tmp_data );

   // 把係數傳回去
  // return ptr_coefficient;
}

// 當 KIND = 2
void
FIR_Window( GtkButton *button, gint mode )
{

   // 這些 widget 是這個函式所需要的
   GtkWidget *frequency_sampling = lookup_widget( GTK_WIDGET( button ), "fs" );
   GtkWidget *pass_band_cutoff = lookup_widget( GTK_WIDGET( button ), "pass_edge" );
   GtkWidget *stop_band_cutoff = lookup_widget( GTK_WIDGET( button ), "stop_edge" );

   //   以下是實現一個FIR( finite impulse response) 的濾波器，產生其頻譜圖其做法
   //   是完成一個加了 window 的低通濾波器，再此我們先行假設其止帶衰減少率為40db
   //   如?馱@來，選擇漢明視窗是較?A當的，而其實所有的視窗都是經驗公式(rule of thumb)
   //   此外，以下的程式中，有一些地方都是應用到：Ωfs = 2πf也就是


   //                數位頻率 × 取樣頻率 ＝ 2 π × 類比頻率

   //      通常來說，一個 FIR 濾波器的設計，其 window 在選用的時候，應該都是根據其
   //   止帶衰減率（stop band attenuation）而選擇的時候，是選擇最接近的止帶衰減率
   //   如果說需求的值僅大於某一視窗的值些許，則必須選擇更大止帶衰減率的視窗，比如說
   //   ，需求要是46 DB，很明顯地，是很接近 Hanning 的45 DB，可是為了要達成有46 DB
   //   的需求則必須要用 Hamming 的視窗，有但是，而其 window 的表格在各大 DSP
   //   數位信號處理的相關書籍?雩茬ㄕ釧狾酗雯苳@些。

   //   以下各個參數是虛擬的。
   //    interval 純粹是?@個圖上的各點在X座標上的間距

   gint i = 0, j = 0;

   // 通帶邊界
   //gfloat pass_band_edge = 2000;

   gfloat pass_band_edge = atof( gtk_entry_get_text( GTK_ENTRY( pass_band_cutoff ) ) );

   // 止帶邊界
   //gfloat stop_band_edge = 3000;

   gfloat stop_band_edge = atof( gtk_entry_get_text( GTK_ENTRY( stop_band_cutoff ) ) );

   // 取樣頻率
   //gfloat fs = 10000;

   gfloat fs = atof( gtk_entry_get_text( GTK_ENTRY( frequency_sampling ) ) );

   //過渡頻帶大小
   gfloat transition_width = stop_band_edge - pass_band_edge ;

   // 此 f_transition_half 值即是頻譜上轉換帶上的“中間值”
   gfloat f_transition_half = pass_band_edge + transition_width / 2;


   //   floor 這個函數是取整數值，且是無條件捨去小數點後面的數字，其代表意義??
   //   在頻譜X座標上面所要取的數目。由於內定假設其止帶衰減率為40 DB ，
   //   故由FIR的視窗表，得知應該使用 Hanning 視窗，其 N 的算法為

   //		    N = 3.32 * 取樣頻率 / 轉換頻帶

   gint N = floor( 3.32 * fs / transition_width );

   // 數位頻率 Ω 的求法，M_PI 是 π，在 math.h 之中有定義
   gfloat omega_transition_half = 2 * M_PI * f_transition_half / fs;

   //   之間的關係為 h = h1 * w ，而 MAX_VALUE 定義在最上面

   gfloat h[ MAX_VALUE ], h1, w ;

   // 在頻譜圖上最大的頻率值，此值相當於Ω = π
   gfloat f_max = fs / 2, omega_max = M_PI;

   // 這個是代表從 0 ～ f_max，或者說是 0 ～ Ω之間其所間隔的頻率，
   // 亦或是想成是在頻率 X 軸上面的間隔

   gfloat f_interval = f_max / pixel, omega_interval = omega_max / pixel;

   gfloat f = 0, omega = 0;

   //   由尤拉式 Euler's identity 可知，其對數的乘冪若為複數（ complex number ）
   //   的話則可使對數型式成為

   //                     cos(θ)- jsin(θ)

   //   而下面的變數宣告之所以要分開是因為 math.h 的函數中沒有處理“對數，且其乘冪為
   //   複數的型式”，在此特別分為實部和虛部，因為在計算其增益大小（ Gain ）的時候，
   //   只是單純把實部（real part），以及虛部（imaginary）兩個部份作完全平方根的處
   //   理，至於若是要研究相位( phase )的部分的話，則要考慮有沒有?t數的問題！

   gfloat H_real = 0, H_imaginary = 0;


   //   此兩個變數，為實部，以及虛部的總和是準備用來做開根號平方和，其結果便為增益Gain
   //   |H(Ω)|，也就是我們的 Y 軸啦！

   gfloat sum_real = 0, sum_imaginary = 0;


   //   gain 和 sum_real 以及 sum_imaginary 有著這樣的關係：

   //                  gain = √( sum_real平方 + sum_imaginary平方 )

   //   gain就是|H(Ω)|，然而 gain_in_db 和 gain 是有著這樣的關係：

   //                  gain_in_db = 20*log( gain )

   //   其實就是聲音的大小，db 是聲音的單位，叫音貝 ( decibels )

   gfloat gain, gain_in_db;


/*
   gfloat coefficient[ MAX_VALUE ];

   gfloat *ptr_coefficient = coefficient;
*/
   //   以下是計算其係數的部份而所謂係數，便是 h ，此 h 等於 w * h1，之所以只讓 i 到
   //   ( N - 1 ) / 2的原因，是因為圖形是一個對稱的增益情形，所以只要有一半的圖形就知
   //   道另一半了！而只到 π 的原因當然因為其函數是 2π 為一個週期。
   for( i = ( ( N - 1 ) / 2 ) * ( -1 ), j = 0; i <= ( N - 1 ) / 2; i++, j++)
   {
      //書上的公式，?~明視窗的公式
      h1 = sin( omega_transition_half * i ) / ( i * M_PI );
      w = 0.5 + 0.5 * cos( ( 2 * M_PI * i ) / ( N - 1 ) );
      h[ j ] = ( h1 * w );
  //    coefficient[ j ] = h[ j ];
      //printf("%d %f \n", j, h[ j ] );
   }


   //   雖然為零值，但是當然不能真的帶0這個值進去，因為分母是不能為 0 的，所以帶入一個
   //   很小的數目，也就是 0 = 0.00001 的意味啦！

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
	       // 表示使用數位頻率表示 X 軸
               H_real = h[ j ] * cos( j * omega );
               H_imaginary = h[ j ] * sin( j * omega );
	       break;
	    }

	    case 3 :
	    case 4 :
	    {
	       // 表示使用類比頻率表示 X 軸
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
            // 表示使用數位頻率表示 X 軸，一般增益表示 Y 軸
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI , gain );

	    // X 軸為數位頻率時的累加
	    omega= omega + omega_interval;

	    break;
	 }
	 case 2 :
	 {
            // 表示使用數位頻率表示 X 軸，音量增益表示 Y 軸
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI, gain_in_db );

	    // X 軸為數位頻率時的累加
	    omega= omega + omega_interval;

	    break;
	 }
	 case 3 :
	 {
	    // 表示使用類比頻率表示 X 軸，一般增益表示 Y 軸
            fprintf( tmp_data, "%f \t %f\n", f, gain );

	    // X 軸為類比頻率時的累加
	    f = f + f_interval;

	    break;
	 }
	 case 4 :
	 {
            // 表示使用類比頻率表示 X 軸，音量增益表示 Y 軸
	    fprintf( tmp_data, "%f \t %f\n", f, gain_in_db );

	    // X 軸為類比頻率時的累加
	    f = f + f_interval;

	    break;
	 }
      }

      fflush( tmp_data );
      sum_real = 0;
      sum_imaginary = 0;
   }
   // 先行關閉 tmp_data 好讓 gnuplot 等一下可以讀這個暫存的資料檔
   fclose( tmp_data );


  // return ptr_coefficient;
}

// KIND = 3
void
IIR_Butterworth( GtkButton *button, gint mode )
{
   // 這些 widget 是這個函式所需要的
   GtkWidget *frequency_sampling = lookup_widget( GTK_WIDGET( button ), "fs" );
   GtkWidget *pass_band_cutoff = lookup_widget( GTK_WIDGET( button ), "pass_edge" );
   GtkWidget *stop_band_cutoff = lookup_widget( GTK_WIDGET( button ), "stop_edge" );
   GtkWidget *stop_band_ripple = lookup_widget( GTK_WIDGET( button ), "stop_ripple" );

   // 以下的參數是用來指定一個 IIR 濾波器的詳細規格，在此是用一個 Butterworth 的例子
   // 在製作一個數位 IIR 濾波器的時候，其首要條件是要先模擬出?@個類比濾波器，然後再用
   // 雙曲線轉換來把類比轉成數位的濾波器

   // 取樣頻率
   //gfloat fs = 8000;

   gfloat fs = atof( gtk_entry_get_text( GTK_ENTRY( frequency_sampling ) ) );

   // 類比濾波器之通帶頻率
   //gfloat analog_fp = 1200;

   gfloat analog_fp = atof( gtk_entry_get_text( GTK_ENTRY( pass_band_cutoff ) ) );

   // 類比濾波器之止帶頻率
   //gfloat analog_fs = 1500;

   gfloat analog_fs = atof( gtk_entry_get_text( GTK_ENTRY( stop_band_cutoff ) ) );

   // 數位濾波器之通帶頻率
   gfloat digital_omega_p = 2 * M_PI * analog_fp / fs;

   // 數位濾波器之止帶頻率
   gfloat digital_omega_s = 2 * M_PI * analog_fs / fs;

   // 經雙曲線轉換的類比通帶頻率
   gfloat wp = 2 * fs * tan( digital_omega_p / 2 );

   // 經雙曲線轉換的類比止帶頻率
   gfloat ws = 2 * fs * tan( digital_omega_s / 2 );

   // 止帶漣波
   //gfloat delta_s = pow( 10. , ( -25. / 20. ) );

   gfloat delta_s = pow( 10. ,
   ( -( atof( gtk_entry_get_text( GTK_ENTRY( stop_band_ripple ) ) ) ) / 20. ) );

   // 數位頻率 Ω = 0 ～ π，相當於類比頻率 f = 0 ～ fs / 2
   gfloat f_max = fs /2, omega_max = M_PI;

   // 由假定的 pixel 中，我們亦可間接計算出 X ?b中上的間隔將為： f_max / pixel ，omega_max / pixel
   gfloat f_interval = f_max / pixel, omega_interval = omega_max / pixel;

   // 每次遞增的 omega 值，其關係為 omega = omega + omega_interval，也就是 X 軸上的點
   // 每次遞增的 f ?�，其關係為 f = f + f_interval，也就是 X 軸上的點
   gfloat f = 0, omega = 0;

   // 階層的計算
   gint n = ceil( log10( 1 / pow( delta_s , 2 ) - 1 ) / ( 2 * log10( ws / wp ) ) );

   // 增益，也就是|H(f)|， 增益以聲音的單位來表示
   gfloat gain, gain_in_db;

   // 在計算增益的時候，其公式太長，其?@部份暫放另一個變數之中
   gfloat temp = 0;

   // 用於迴圈的累計
   gint i ;

   // 為什麼 i < pixel - 1 呢？因為當 tan( π / 2 ) 的時候，結果是無限大，而事實上，在當 θ 在很接近
   // π / 2 的時候，已經是非常大的數字，幾乎接近 float 的範圍?A所以為了不去碰到那個界限，不要去算他是比
   // 較安全的啦！
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
            // 此算法是以 X 軸為數位頻率時的算法
            temp = pow( 2 * fs * tan( omega / 2 ) / wp, 2 * n );

	    break;
	 }
	 case 3 :
	 case 4 :
         {
	    // 此算法是以 X 軸為類比頻率時的算法
            temp = pow( 2 * fs * tan( M_PI * f / fs ) / wp, 2 * n );

	    break;
	 }
      }

      // 此為一般的表示增益
      gain = 1 / pow( temp + 1, 0.5 );

      // 此為?H聲音的大小表示增益
      gain_in_db = 20 * log10( gain );

      switch( mode )
      {
         case 0 :
	 case 1 :
	 {
            // 表示使用數位頻率表示 X 軸，一般增益表示 Y 軸
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI , gain );

	    // X 軸為數位頻率時的累加
	    omega= omega + omega_interval;

	    break;
	 }
	 case 2 :
	 {
            // 表示使用數位頻率表示 X 軸，音量增益表示 Y 軸
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI, gain_in_db );

	    // X 軸為數位頻率時的累加
	    omega= omega + omega_interval;

	    break;
	 }
	 case 3 :
	 {
	    // 表示使用類比頻率表示 X 軸，一般增益表示 Y 軸
            fprintf( tmp_data, "%f \t %f\n", f, gain );

	    // X 軸為類比頻率時的累加
	    f = f + f_interval;

	    break;
	 }
	 case 4 :
	 {
            // 表示使用類比頻率表示 X 軸，音量增益表示 Y 軸
	    fprintf( tmp_data, "%f \t %f\n", f, gain_in_db );

	    // X 軸為類比頻率時的累加
	    f = f + f_interval;

	    break;
	 }
      }

      fflush( tmp_data );

   }

   // 先行關閉 tmp_data 好讓 gnuplot 等一下可以讀這個暫存的資料檔
   fclose( tmp_data );
}

// KIND = 4
void
IIR_Chebyshev( GtkButton *button, gint mode )
{
   // 這些 widget 是這個函式所需要的
   GtkWidget *frequency_sampling = lookup_widget( GTK_WIDGET( button ), "fs" );
   GtkWidget *pass_band_cutoff = lookup_widget( GTK_WIDGET( button ), "pass_edge" );
   GtkWidget *stop_band_cutoff = lookup_widget( GTK_WIDGET( button ), "stop_edge" );
   GtkWidget *pass_band_ripple = lookup_widget( GTK_WIDGET( button ), "pass_ripple" );
   GtkWidget *stop_band_ripple = lookup_widget( GTK_WIDGET( button ), "stop_ripple" );

   // 以下的參數是用來指定一個 IIR 濾波器的詳細規格，在此是用一個 Chebyshev Type I
   // 的例子在製作一個數?? IIR 濾波器的時候，其首要條件是要先模擬出一個類比濾波器，
   // 然後再用雙曲線轉換來把類比轉成數位的濾波器

   // 取樣頻率
   //gfloat fs = 20000;

   gfloat fs = atof( gtk_entry_get_text( GTK_ENTRY( frequency_sampling ) ) );

   // 類比濾波器之通帶頻率
   //gfloat analog_fp = 5000;

   gfloat analog_fp = atof( gtk_entry_get_text( GTK_ENTRY( pass_band_cutoff ) ) );

   // 類比濾波器之?豏a頻率
   //gfloat analog_fs = 7500;

   gfloat analog_fs = atof( gtk_entry_get_text( GTK_ENTRY( stop_band_cutoff ) ) );

   // 數位濾波器之通帶頻率
   gfloat digital_omega_p = 2 * M_PI * analog_fp / fs;

   // 數位濾波器之止帶頻率
   gfloat digital_omega_s = 2 * M_PI * analog_fs / fs;

   // 經雙曲線轉換的類比通帶頻率
   gfloat wp = 2 * fs * tan( digital_omega_p / 2 );

   // 經雙曲線轉換的類比止帶頻率
   gfloat ws = 2 * fs * tan( digital_omega_s / 2 );

   // 通帶漣波
   //gfloat delta_p = pow( 10. , ( -1. / 20. ) );

   gfloat delta_p = pow( 10. ,
   ( -( atof( gtk_entry_get_text( GTK_ENTRY( pass_band_ripple ) ) ) )/ 20. ) );

   // 由通帶漣波所決定之參數ε
   gfloat epsilon = sqrt( pow( delta_p, -2 ) - 1 );

   // Chebyshev polynomial 契比雪夫多項式的值
   gfloat chebyshev;

   // 止帶漣波δ
   // gfloat delta_s = pow( 10. , ( -32. / 20. ) );
   gfloat delta_s = pow( 10. ,
   ( -( atof( gtk_entry_get_text( GTK_ENTRY( stop_band_ripple ) ) ) ) / 20. ) );

   // 由止帶漣波δ所決定的參數δ
   gfloat delta = sqrt( pow( delta_s, -2 ) - 1 );

   // 數位頻率 Ω = 0 ～ π，相當於類比頻率 f = 0 ～ fs / 2
   gfloat omega_max = M_PI, f_max = fs / 2;

   // 由假定的 pixel 中，我們亦可間接計算出 X 軸中上的間隔將為： omega_max / pixel ，f_max / pixel
   gfloat omega_interval = omega_max / pixel, f_interval = f_max / pixel;

   // 每次遞增的 omega 值，其關係為 omega = omega + omega_interval，也就是 X 軸上的點
   // 每次遞增的 f 值，其關係為 f = f + f_interval，也就是 X 軸上的點
   gfloat omega = 0, f = 0;

   // 階層的計算
   gint n = ceil( acosh( delta / epsilon ) / acosh( ws / wp ) );

   // 增益，也就是|H(f)|, 增益以聲音的單位來表示
   gfloat gain, gain_in_db;

   // 在計算增益的時候，其公式太長，其一部份暫放另一個陣列之中
   gfloat temp = 0;

   // 用於迴圈的累計
   gint i ;

   // 數位濾波器在頻譜上的 X 跟 Y 的求值後，寫入檔案指標 tmp_data 之中，

   // 為什麼 i < pixel - 1 呢？因為當 tan( π / 2 ) 的時候，結果是無限大，而事實上，在當 θ 在很接近
   // π / 2 的時候，已經是非常大的數字，幾乎接近 float 的範圍，所以為了不去碰到那個界限，不要去算他是比
   // 較安全的啦！
   //for( i = 0; i <= pixel; i++)
   for( i = 0; i < pixel - 1; i++)
   {
      switch( mode )
      {
         case 0 :
	 case 1 :
	 case 2 :
         {
            // 此算法是以 X 軸為數位頻率時的算法
            temp = 2 * fs * tan( omega / 2 ) / wp;

	    break;
	 }
	 case 3 :
	 case 4 :
         {
            // 而 X 是以“類比頻率”表示
            temp = 2 * fs * tan( M_PI * f / fs ) / wp;

	    break;
	 }
      }
      // 因為 abs 這個函式只能用在整數的參數，所以無法在此使用
      if( temp < 0 )
         temp = temp * ( -1 ) ;

      // 下面的判斷式就是契比雪夫的多項式，當 temp 的絕對值大於1時，和小於1時的不同
      if( temp > 1. )
         chebyshev = cosh( n * acosh( temp ) );
      else
         chebyshev =  cos( n *  acos( temp ) );

      // 此為一般的表示增益
      gain = pow( 1 + pow( epsilon, 2 ) * pow( chebyshev, 2 ) , -0.5 );

      // 此為以聲音的大小表示增益
      gain_in_db = 20 * log10( gain );

      switch( mode )
      {
         case 0 :
	 case 1 :
	 {
            // 表示使用數位頻率表示 X 軸，一般增益表示 Y 軸
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI , gain );

	    // X 軸為數位頻率時的累加
	    omega= omega + omega_interval;

	    break;
	 }
	 case 2 :
	 {
            // 表示使用數位頻率表示 X 軸，音量增益表示 Y 軸
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI, gain_in_db );

	    // X 軸為數位頻率時的累加
	    omega= omega + omega_interval;

	    break;
	 }
	 case 3 :
	 {
	    // 表示使用類比頻率表示 X 軸，一般增益表示 Y 軸
            fprintf( tmp_data, "%f \t %f\n", f, gain );

	    // X 軸為類比頻率時的累加
	    f = f + f_interval;

	    break;
	 }
	 case 4 :
	 {
            // 表示使用類比頻率表示 X 軸，音量增益表示 Y 軸
	    fprintf( tmp_data, "%f \t %f\n", f, gain_in_db );

	    // X 軸為類比頻率時的累加
	    f = f + f_interval;

	    break;
	 }
      }

      fflush( tmp_data );
   }

   // 先行關閉 tmp_data 好讓 gnuplot 等一下可以讀這個暫存的資料檔
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

   // 在頻譜圖上最大的頻率值，此值相當於 Ω = π
   gfloat f_max = fs / 2, omega_max = M_PI;

   // 這個是代表從 0 ～ f_max，或者說是 0 ～ Ω之間其所間隔的頻率，
   // 亦或是想成是在頻率 X 軸上面的間隔

   gfloat f_interval = f_max / pixel, omega_interval = omega_max / pixel;

   gfloat f = 0, omega = 0;

   //   由尤拉式 Euler's identity 可知，其對數的乘冪若為複數（ complex number ）
   //   的話則可使對數型式成為

   //                     cos(θ)- jsin(θ)

   //   而下面的變數宣告之所以要分開是因為 math.h 的函數中沒有處理“對數，且其乘冪為
   //   複數的型式”，在此特別分?偎窸〝M虛部，因為在計算其增益大小（ Gain ）的時候，
   //   只是單純把實部（real part），以及虛部（imaginary）兩個部份作完全平方根的?B
   //   理，至於若是要研究相位( phase )的部分的話，則要考慮有沒有?t數的問題！

   gfloat H_real_a = 0, H_imaginary_a = 0, H_real_b = 0, H_imaginary_b = 0;


   //   此兩個變數，為實部，以及虛部的總和是準備用來做開根號平方和，其結果便為增益Gain
   //   |H(Ω)|，也就是我們的 Y 軸啦！

   gfloat sum_real_a = 0, sum_imaginary_a = 0, sum_real_b = 0, sum_imaginary_b = 0;

   //   gain 和 sum_real 以及 sum_imaginary 有著這樣的關係：

   //                  gain = √( sum_real平方 + sum_imaginary平方 )

   //   gain就是|H(Ω)|，然而 gain_in_db 和 gain 是有著這樣的關係：

   //                  gain_in_db = 20*log( gain )

   //   其實就是聲音的大小，db 是聲音的單位，叫音貝 ( decibels )

   gfloat gain, gain_in_db;

   // 用來放要貯存資料的路徑檔名
   gchar *path;

   // 將是一個要被寫入資料檔的檔名指標
   FILE *file = NULL;

   gchar str[80];

   // 判斷是否為未處理的資料，TRUE 則為“是”
   gboolean crude = FALSE;

   // 因為可能有人的資料實在大到九十九萬，所以一定要用 long
   glong raw_data[ MAX_VALUE ],ripe_data[ MAX_VALUE ];

   // 因為數值資料總加之後很有可能會超過 int 的界限，所以一定要用  gulong
   gulong sum_raw = 0, sum_ripe = 0;

   gfloat average_raw, average_ripe;

   // 表示讓使用者可以用“,”、“;”或者是“空白鍵”來分隔轉移函數中的係數，
   // 而 segment 代表被分割的各個部份然後再一一存放在各個陣列位置
   // 而 substitute 是暫時將 entry 中係數暫放在此處
   gchar *sift = ",; ", *segment = "", substitute[ MAX_VALUE ];

   // term 代表項數，也就是幾階的意思，也是係數的數目啦！
   gint term_a = 0, term_b = 0;

   // 可以放置係數的陣列
   gfloat a_factor[ MAX_VALUE ], b_factor[ MAX_VALUE ];

   gint i = -1, j = -1;
//////////////////////////////
//float data_xn[ MAX_VALUE ] = { 10, 14, 16, 15, 11, 14, 12 };

//float sum_xn = 0, average_xn, sum_yn = 0, average_yn;

gfloat sum_yn = 0, average_yn;
   // 七筆資料
gint amount = 499;

gfloat yn[ MAX_VALUE ], xn[ MAX_VALUE ];
////////////////////////////////
   system( "clear" );


   // path 放著要寫入的資料檔路徑及其檔案
   path = gtk_entry_get_text( GTK_ENTRY( saveaspath ) );

   file = fopen( path, "r" );

   // 確保檔案的讀寫位置確實在檔頭的位置
   rewind( file );

   while( fgets( str, SIZE, file ) )
   {
      // 表示交換輪流讀取“生”資料及“已濾波”的資料並且把讀來的資料放入陣列之中
      if( ( crude = !crude ) )
         raw_data[ ++i ] = atoi( str );
      else
         ripe_data[ ++j ] = atoi( str );

      // 這個函式是把目前所指的檔案讀寫位置依“相對位置”的方式，偏移幾個?鴗葡捸A以便於找下一個資料讀取
      fseek( file, 1, SEEK_CUR );
   }

   // 用完就立刻關起來，確保安全
   fclose( file );

 //  for( i = 0 ; i<=499; i++)
   {
  //   g_print("%d raw_data = %ld \t ripe_data = %ld \n",i,raw_data[ i ],ripe_data[ i ]);
  //   getchar();
   }

   // 此時資料的總數自然就可以確定了
   amount = i;

   // 算出原始資料的總和
   for( i = 0; i <= amount; i++)
   {
      sum_raw = sum_raw + raw_data[ i ];
      sum_ripe = sum_ripe + ripe_data[ i ];
   }

//   g_print("sum_raw = %ld \t sum_ripe = %ld \n",sum_raw,sum_ripe );

   // 算出平均值，轉換為 float 才可以有小數出現。
   average_raw  = ( gfloat )sum_raw  / ( gfloat )( amount + 1 );
   average_ripe = ( gfloat )sum_ripe / ( gfloat )( amount + 1 );

//   g_print("average_raw = %f \t average_ripe = %f \n",average_raw,average_ripe );


   // 確定是選擇 IIR 的濾波器，如果是，則會需要有用到 a 的係數，而 b 的係數是不管哪一種
   // 濾波器都一定要的啦！
   if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( iir_input_coefficient ) ) )
   {
      // 不直接把 gtk_entry_get_text( GTK_ENTRY( a_value ) ) 作 strtok 分解的動作
      // 其原因是因為，我發覺當程式中的 entry 值只有在被“改變數值”的時候，其
      // gtk_entry_get_text( GTK_ENTRY( a_value ) ) 才會得到另一個新的值，
      // 換句話說，也就是平常的時候電腦運作的時候是把 gtk_entry_get_text( GTK_ENTRY( a_value ) )
      // 暫時放到某個記憶體區，如果你的鍵盤沒有在那個 entry 裡面造成任何類似 key press 的動作的話，
      // 事實上，在 gtk_entry_get_text( GTK_ENTRY( a_value ) ) 指令之後，仍然是抓取原來
      // 存區的值，也就是沒有真的去抓 entry 裡面的值啦！
      // 而本程式中如果沒有去改變 entry 裡面的值，而因為用到 strtok 這個函式，所以他只會一直去那
      // 暫存區的值，為了避免這種情形，就先決定放置在一個 substitute 這個陣列中，
      strcpy( substitute, gtk_entry_get_text( GTK_ENTRY( a_value ) ) );

      a_factor[ term_a ] = atof( strtok( substitute, sift ) );

      while( ( segment = strtok( NULL, sift ) ) )
      {
         term_a++;
         a_factor[ term_a ] = atof( segment );
      }
   }
   // g_print("term_a = %d\n", term_a );

   // 作法及其理由和 a_value 一樣
   strcpy( substitute, gtk_entry_get_text( GTK_ENTRY( b_value ) ) );
   b_factor[ term_b ] = atof( strtok( substitute , sift ) );

   while( ( segment = strtok( NULL, sift ) ) )
   {
      term_b++;
      b_factor[ term_b ] = atof( segment );
   }
   // g_print("term_b = %d\n", term_b );

////////////////////////
   // 算出原始 data 的總和
  // for( i = 0; i <= amount; i++)
  //    sum_xn = sum_xn + data_xn[ i ];
   //g_print("sum_xn = %f\n", sum_xn );

   // 算出平均值
//   average_xn = sum_xn  / ( amount + 1 );
   //g_print("average_xn = %f\n", average_xn );

   // 把要計算的 xn 值做一個初始化的動作
   for( i = 0; i <= term_b ; i++ )
      xn[ i ] = average_raw;

   xn[ 0 ] = raw_data[ 0 ];

   // 有多少輸入，就有多少輸出
   for( i = 0; i <= amount; i++ )
   {
      // 先清乾淨，以策安全
      yn[ i ] = 0;

      // 倒著做回來，也就是從最後一項開始做，到第一項
      for( j = term_b; j >= 0; j-- )
      {
         // 各項的累加
         yn[ i ] = yn[ i ] + b_factor[ j ] * xn[ j ];

         if( j == 0 )
            // 代表已經算到第一項了，表示此次的輸出即將結束
	    xn[ j ] = raw_data[ i + 1 ];
	 else
	    // 每次都要向右移動一位
	    xn[ j ] = xn[ j - 1 ];
      }

      sum_yn = sum_yn + yn[ i ];
      //g_print("y[%d] = %f\n",i, yn[ i ] );
   }

   // 算出平均值
   average_yn = sum_yn  / ( amount + 1 );
   // printf("%f\n",average_yn );

////////////

   // 開始計算 X 軸的各點
   for( i = 0; i < pixel; i++ )
   {

      // 判斷是否需要用到 a 係數
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
	          // 表示使用數位頻率表示 X 軸
                  H_real_a = a_factor[ j ] * cos( j * omega );
                  H_imaginary_a = a_factor[ j ] * sin( j * omega );
	          break;
	       }

	       case 3 :
	       case 4 :
	       {
	          // 表示使用類比頻率表示 X 軸
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
	       // 表示使用數位頻率表示 X 軸
//               H_real_b = b_factor[ j ] * cos( j * omega );
//               H_imaginary_b = b_factor[ j ] * sin( j * omega );
//	       break;
	    }

//            case 3 :
//	    case 4 :
	    {
	       // 表示使用類比頻率表示 X 軸
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
	       // 表示使用數位頻率表示 X 軸
               H_real_b = ( yn[ j ] - average_yn ) * cos( j * omega );
               H_imaginary_b = ( yn[ j ] - average_yn ) * sin( j * omega );
	       break;
	    }

            case 3 :
	    case 4 :
	    {
	       // 表示使用類比頻率表示 X 軸
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
            // 表示使用數位頻率表示 X 軸，一般增益表示 Y 軸
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI , gain );

	    // X 軸為數位頻率時的累加
	    omega= omega + omega_interval;

	    break;
	 }
	 case 2 :
	 {
            // 表示使用數位頻率表示 X 軸，音量增益表示 Y 軸
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI, gain_in_db );

	    // X 軸為數位頻率時的累加
	    omega= omega + omega_interval;

	    break;
	 }
	 case 3 :
	 {
	    // 表示使用類比頻率表示 X 軸，一般增益表示 Y 軸
            fprintf( tmp_data, "%f \t %f\n", f, gain );

	    // X 軸為類比頻率時的累加
	    f = f + f_interval;

	    break;
	 }
	 case 4 :
	 {
            // 表示使用類比頻率表示 X 軸，音量增益表示 Y 軸
	    fprintf( tmp_data, "%f \t %f\n", f, gain_in_db );

	    // X 軸為類比頻率時的累加
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

   // 先行關閉 tmp_data 好讓 gnuplot 等一下可以讀這個暫存的資料檔
   fclose( tmp_data );

}


//KIND = 6
void
protodata( GtkButton *button, gint mode )
{
   GtkWidget *saveaspath = lookup_widget( GTK_WIDGET( button ), "entry1" );
   GtkWidget *frequency_sampling = lookup_widget( GTK_WIDGET( button ), "fs" );

   // 用來放要貯存資料的路徑檔名
   gchar *path;

   gint i = -1;

   gint j = -1;

   gint amount;

   // 因為數值資料總加之後很有可能會超過 int 的界限，所以一定要用  gulong
   gulong sum_raw = 0, sum_ripe = 0;

   gfloat average_raw, average_ripe;

   gfloat fs = atof( gtk_entry_get_text( GTK_ENTRY( frequency_sampling ) ) );

   // 在頻譜圖上最大的頻率值，此值相當於 Ω = π
   gfloat f_max = fs / 2, omega_max = M_PI ;

   // 這個是代表從 0 ～ f_max，或者說是 0 ～ Ω之間其所間隔的頻率，
   // 亦或是想成是在頻率 X 軸上面的間隔

   gfloat f_interval = f_max / pixel, omega_interval = omega_max / pixel;

   gfloat f = 0, omega = 0;


   //   由尤拉式 Euler's identity 可知，其對數的乘冪若為複數（ complex number ）
   //   的話則可使對數型式成為

   //                     cos(θ) - jsin(θ)

   //   而下面的變數宣告之所以要分開是因為 math.h 的函數中沒有處理“對數，且其乘冪為
   //   複數的型式”，在此特別分為實部和虛部，因為在計算其增益大小（ Gain ）的時候，
   //   只是單純把實部（real part），以及虛部（imaginary）兩個部份作完全平方根的處
   //   理，至於若是要研究相位( phase )的部分的話，則要考慮有沒有負數的問題！

   gfloat H_real = 0, H_imaginary = 0;

   //   此兩個變數，為實部，以及虛部的總和是準備用來做開根號平方和，其結果便為增益Gain
   //   |H(Ω)|，也就是我們的 Y 軸啦！

   gfloat sum_real = 0, sum_imaginary = 0;


   //   gain 和 sum_real 以及 sum_imaginary 有著這樣的關係：

   //                  gain = √( sum_real平方 + sum_imaginary平方 )

   //   gain就是|H(Ω)|，然而 gain_in_db 和 gain 是有著這樣的關係：

   //                  gain_in_db = 20*log( gain )

   //   其實就是聲音的大小，db 是聲音的單位，叫音貝 ( decibels )

   gfloat gain, gain_in_db;

   // 將是一個要被寫入資料檔的檔名指標
   FILE *file = NULL;

   gchar str[80];

   // 因為可能有人的資料實在大到九十九萬，所以一定要用 long
   glong raw_data[ MAX_VALUE ],ripe_data[ MAX_VALUE ];

   // 判斷是否為未處理的資料，TRUE 則為“是”
   gboolean crude = FALSE;

   // 頻譜圖中的總值，若是平滑度取的不一樣，則值會改變喔！
   total = 0;

   // 頻譜圖中的最大值
   max = 0;

   // path 放著要寫入的資料檔路徑及其檔案
   path = gtk_entry_get_text( GTK_ENTRY( saveaspath ) );

   file = fopen( path, "r" );

   // 確保檔案的讀寫位置確實在檔頭的位置
   rewind( file );

   while( fgets( str, SIZE, file ) )
   {
      // 表示交換輪流讀取“生”資料及“已濾波”的資料並且把讀來的資料放入陣列之中
      if( ( crude = !crude ) )
         raw_data[ ++i ] = atoi( str );
      else
         ripe_data[ ++j ] = atoi( str );

      // 這個函式是把目前所指的檔案讀寫位置依“相對位置”的方式，偏移幾個?鴗葡捸A以便於找下一個資料讀取
      fseek( file, 1, SEEK_CUR );
   }

   // 用完就立刻關起來，確保安全
   fclose( file );

  // for( i = 0 ; i<=499; i++)
  //   g_print("raw_data = %ld \t ripe_data = %ld \n",raw_data[ i ],ripe_data[ i ]);

   // 此時資料的總數自然就可以確定了
   amount = i;

   // 算出原始資料的總和
   for( i = 0; i <= amount; i++)
   {
      sum_raw = sum_raw + raw_data[ i ];
      sum_ripe = sum_ripe + ripe_data[ i ];
   }

   //g_print("sum_raw = %ld \t sum_ripe = %ld \n",sum_raw,sum_ripe );

   // 算出平均值，轉換為 float 才可以有小數出現。
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
	       // 表示使用數位頻率表示 X 軸

      //         H_real = ( raw_data[ j ] - average_raw ) * cos( j * omega );
     //          H_imaginary = ( raw_data[ j ] - average_raw ) * sin( j * omega );

               H_real = ( ripe_data[ j ] - average_ripe ) * cos( j * omega );
               H_imaginary = ( ripe_data[ j ] - average_ripe ) * sin( j * omega );
	       break;
	    }

	    case 3 :
	    case 4 :
	    {
               // 表示使用類比頻率表示 X 軸

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

      // 作加總增益及取最大值的動作
      switch( mode )
      {
         case 0 :
	 case 1 :
	 case 3 :
         {
            // 頻譜圖上的加總值
            total = total + gain;

            // 求得最大的線性增益值
            if( max < gain )
               max = gain;
	    break;
	 }

	 case 2 :
	 case 4 :
	 {
            // 頻譜圖上的加總值
            total = total + gain_in_db;

            // 求得最大的分貝增益值
            if( max < gain_in_db )
               max = gain_in_db;
            break;
	 }
      }

      // 使用類比頻率表示 X 軸
      switch( mode )
      {
         case 0 :
	 case 1 :
	 {
            // 表示使用數位頻率表示 X 軸，一般增益表示 Y 軸
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI , gain );

	    // X 軸為數位頻率時的累加
	    omega= omega + omega_interval;

	    break;
	 }
	 case 2 :
	 {
            // 表示使用數位頻率表示 X 軸，音量增益表示 Y 軸
	    fprintf( tmp_data, "%f \t %f\n", omega / M_PI, gain_in_db );

	    // X 軸為數位頻率時的累加
	    omega= omega + omega_interval;

	    break;
	 }
	 case 3 :
	 {
	    // 表示使用類比頻率表示 X 軸，一般增益表示 Y 軸
            fprintf( tmp_data, "%f \t %f\n", f, gain );

	    // X 軸為類比頻率時的累加
	    f = f + f_interval;

	    break;
	 }
	 case 4 :
	 {
            // 表示使用類比頻率表示 X 軸，音量增益表示 Y 軸
	    fprintf( tmp_data, "%f \t %f\n", f, gain_in_db );

	    // X 軸為類比頻率時的累加
	    f = f + f_interval;

	    break;
	 }
      }

      fflush( tmp_data );
      sum_real = 0;
      sum_imaginary = 0;
   }

  // g_print("total = %f ,max = %f\n", total ,max);

   // 先行關閉 tmp_data 好讓 gnuplot 等一下可以讀這個暫存的資料檔
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
   // 明細濾波器的輸入元件
   GtkWidget *move_average = lookup_widget( GTK_WIDGET( button ), "move_average");
   GtkWidget *window = lookup_widget( GTK_WIDGET( button ),       "window"      );
   GtkWidget *butterworth = lookup_widget( GTK_WIDGET( button ),  "butterworth" );
   GtkWidget *chebyshev = lookup_widget( GTK_WIDGET( button ),    "chebyshev"   );
   GtkWidget *pass_edge   = lookup_widget( GTK_WIDGET( button ),   "pass_edge" );
   GtkWidget *stop_edge   = lookup_widget( GTK_WIDGET( button ),   "stop_edge" );
   GtkWidget *pass_ripple = lookup_widget( GTK_WIDGET( button ), "pass_ripple" );
   GtkWidget *stop_ripple = lookup_widget( GTK_WIDGET( button ), "stop_ripple" );

   // 要用假設的轉移函數係數輸入的元件
   GtkWidget *fir_input_coefficient = lookup_widget( GTK_WIDGET( button ), "fir_input_coefficient" );
   GtkWidget *iir_input_coefficient = lookup_widget( GTK_WIDGET( button ), "iir_input_coefficient" );
   GtkWidget *a_value = lookup_widget( GTK_WIDGET( button ), "a_value" );
   GtkWidget *b_value = lookup_widget( GTK_WIDGET( button ), "b_value" );

   // 明細濾波器的元件全部打開
   gtk_widget_set_sensitive( GTK_WIDGET( move_average ), TRUE );
   gtk_widget_set_sensitive( GTK_WIDGET( window       ), TRUE );
   gtk_widget_set_sensitive( GTK_WIDGET( butterworth  ), TRUE );
   gtk_widget_set_sensitive( GTK_WIDGET( chebyshev    ), TRUE );
   gtk_widget_set_sensitive( GTK_WIDGET( pass_edge ), TRUE );
   gtk_widget_set_sensitive( GTK_WIDGET( stop_edge ), TRUE );
   gtk_widget_set_sensitive( GTK_WIDGET( pass_ripple ), TRUE );
   gtk_widget_set_sensitive( GTK_WIDGET( stop_ripple ), TRUE );

   // 假設的轉移函數係數輸入的元件，全部失能
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

   // 明細濾波器的輸入元件
   GtkWidget *move_average = lookup_widget( GTK_WIDGET( button ), "move_average");
   GtkWidget *window = lookup_widget( GTK_WIDGET( button ),       "window"      );
   GtkWidget *butterworth = lookup_widget( GTK_WIDGET( button ),  "butterworth" );
   GtkWidget *chebyshev = lookup_widget( GTK_WIDGET( button ),    "chebyshev"   );
   GtkWidget *pass_edge   = lookup_widget( GTK_WIDGET( button ),   "pass_edge" );
   GtkWidget *stop_edge   = lookup_widget( GTK_WIDGET( button ),   "stop_edge" );
   GtkWidget *pass_ripple = lookup_widget( GTK_WIDGET( button ), "pass_ripple" );
   GtkWidget *stop_ripple = lookup_widget( GTK_WIDGET( button ), "stop_ripple" );

   // 要用假設的轉移函數係數輸入的元件
   GtkWidget *fir_input_coefficient = lookup_widget( GTK_WIDGET( button ), "fir_input_coefficient"      );
   GtkWidget *iir_input_coefficient = lookup_widget( GTK_WIDGET( button ), "iir_input_coefficient" );
   GtkWidget *a_value = lookup_widget( GTK_WIDGET( button ), "a_value" );
   GtkWidget *b_value = lookup_widget( GTK_WIDGET( button ), "b_value" );

   // 明細濾波器的元件全部關閉
   gtk_widget_set_sensitive( GTK_WIDGET( move_average ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( window       ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( butterworth  ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( chebyshev    ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( pass_edge ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( stop_edge ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( pass_ripple ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( stop_ripple ), FALSE );

   // 假設的轉移函數係數輸入的元件，全部致能
   gtk_widget_set_sensitive( GTK_WIDGET( fir_input_coefficient ), TRUE );
   gtk_widget_set_sensitive( GTK_WIDGET( iir_input_coefficient ), TRUE );
   gtk_widget_set_sensitive( GTK_WIDGET( a_value ), TRUE );
   gtk_widget_set_sensitive( GTK_WIDGET( b_value ), TRUE );



   statusbar = lookup_widget( GTK_WIDGET( button ), "statusbar1" );

   // 把訊息填入狀態列中
   gtk_statusbar_push( GTK_STATUSBAR( statusbar ), 1, "請用“空白鍵”或“;”“,”分隔係數，若有缺項，煩請自行補“0”" );

   // 假設之轉移函數的計算都在 KIND = 5 裡面計算出來
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

   // 把不需要的參數關掉，失能
   gtk_widget_set_sensitive( GTK_WIDGET( stop_edge   ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( pass_ripple ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( stop_ripple ), FALSE );

   // 放入預設值
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

   // 把不需要的參數關掉，失能，需要的參數打開，致能
   gtk_widget_set_sensitive( GTK_WIDGET( stop_edge   ), TRUE  );
   gtk_widget_set_sensitive( GTK_WIDGET( pass_ripple ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( stop_ripple ), FALSE );

   // 放入預設值
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

   // 把不需要的參數關掉，失能，需要的參數打開，致能
   gtk_widget_set_sensitive( GTK_WIDGET( stop_edge   ), TRUE  );
   gtk_widget_set_sensitive( GTK_WIDGET( pass_ripple ), FALSE );
   gtk_widget_set_sensitive( GTK_WIDGET( stop_ripple ), TRUE  );

   // 放入預設值
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

   // 把需要的參數打開，致能
   gtk_widget_set_sensitive( GTK_WIDGET( stop_edge   ), TRUE  );
   gtk_widget_set_sensitive( GTK_WIDGET( pass_ripple ), TRUE );
   gtk_widget_set_sensitive( GTK_WIDGET( stop_ripple ), TRUE );

   // 放入預設值
   gtk_entry_set_text(GTK_ENTRY ( fs ), "20000" );
   gtk_entry_set_text(GTK_ENTRY ( pass_edge ), "5000" );
   gtk_entry_set_text(GTK_ENTRY ( stop_edge ), "7500" );
   gtk_entry_set_text(GTK_ENTRY ( pass_ripple ), "1" );
   gtk_entry_set_text(GTK_ENTRY ( stop_ripple ), "32" );

   KIND  = 4;
}


// toggled 的事件是會發生在同一組中狀態改變的時候，比如有 A 、B 兩個 radiobutton ，而預設是 A 為
// 致能，如果一旦 radiobutton B 被點選的時候，則 radiobutton A ?? toggled 事件會先被啟動，而
// radiobutton B 的 toggle 事件才會被啟動，也就是說，在英文當中， toggle 的意思是類似 switch 的
// 東西，?P一組當中只要任何有發生“開”或“關”動作者，一律都會啟動其 toggled 事件，這也就是為什麼每一個
// toggled 事件在此都要加入 if( gtk_toggle_button_get_active( togglebutton ) ) 以確保目前的
// 事件是被 enabled 而?眶o的，而不是因 disable 而觸發的
// 此外，在運用 radiobutton 的時候，記得要把 group 的值定義好，同類的歸在一起用同一個 group ID 的
// 而如果你的視窗上只有這樣一組?? radiobutton 的時候，則可不加 group ID 的，因為系統會認定是同一組
// 不過養成好習慣，對你總是好的，不要老是依靠“自動化”作業


void
on_fir_input_coefficient_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
   GtkWidget *a_value = lookup_widget( GTK_WIDGET( button ), "a_value" );
   GtkWidget *b_value = lookup_widget( GTK_WIDGET( button ), "b_value" );
   GtkWidget *fs          = lookup_widget( GTK_WIDGET( button ),   "fs"        );

   // 抓取 optionmenu1 的目前值，以便決定 X 、 Y 軸用何種方式表現
   GtkWidget *optionmenu1 = lookup_widget( GTK_WIDGET( button ), "optionmenu1" );
   GtkWidget *menu = GTK_OPTION_MENU( optionmenu1 ) -> menu;
   GtkWidget *active_item = gtk_menu_get_active( GTK_MENU( menu ) );
   gint mode = g_list_index( GTK_MENU_SHELL( menu ) -> children, active_item );


   // 因為 FIR 的差分方程式是沒有 a 的係數，換句話說，也就是沒有過去的輸出
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

   // 抓取 optionmenu1 的目前值，以便決定 X 、 Y 軸用何種方式表現
   GtkWidget *optionmenu1 = lookup_widget( GTK_WIDGET( button ), "optionmenu1" );
   GtkWidget *menu = GTK_OPTION_MENU( optionmenu1 ) -> menu;
   GtkWidget *active_item = gtk_menu_get_active( GTK_MENU( menu ) );
   gint mode = g_list_index( GTK_MENU_SHELL( menu ) -> children, active_item );

   // 因為 FIR 差分方程式中沒有 a 的係數，換句話說，也就是沒?章L去的輸出
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

   // 確定目前是否是要使用印表機來列印頻譜圖
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

   // 丟到背景去做，就不會使主程式僵死。
   sprintf( sys_command, "gv %s &", file_path );
   
   // system 這個函式可以將參數，丟到終端機底下去執行
   system( sys_command );

}


