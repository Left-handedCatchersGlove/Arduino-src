/**
 * @brief シリアル通信用プログラム（受信のみ）
 **/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

HANDLE h;  // ハンドルの定義

void main()
{
  int i = 0;
  char sBuf[1];
  char str[100];
  int baudRate = 9600;
  unsigned long nn;
  DCB dbc;
  COMMTIMEOUTS cto;

  /*-------- ファイルのクリエイト/オープン --------*/
  h = CreateFile( "COM1",      // ポート」の名前
                  GENERIC_READ | GENERIC_WRITE, // Read/Write
                  0,   // 共有しない
                  0,   // セキュリティ属性デフォルト
                  OPEN_EXISTING, // 既存デフォルト
                  0,   // 非同期I/Oを許す
                  0 );

  if( h == INVALID_HANDLE_VALUE ) // ハンドル番号が戻ってこないとき
  {
    printf("Open Error!\n");
    exit(1);
  }

  /*-------- シリアルポートの状態操作 --------*/
  GetCommState( h, &dcb ); // シリアルポートの状態を取得
  dcb.BaudRate = baudRate;
  SetCommState( h, &dcb ); // シリアルポートの状態を設定

  /*-------- シリアルポートのタイムアウト状態操作 --------*/
  GetCommTimeouts( h, &cto ); // タイムアウト設定状態を取得
  cto.ReadIntervalTimeout = 1000;
  cto.ReadTotalTimeoutMultiplier = 0;
  cto.ReadTotalTimeoutConstant = 1000;
  cto.WriteTotalTimeoutMultiplier = 0;
  cto.WriteTotalTimeoutConstant = 0;
  SetCommTimeouts( h, &cto ); // タイムアウト状態を設定

  /*-------- 受信データの読み込み（1行分の文字列） --------*/
  while(1)
  {
    ReadFile( h, sBuf, 1, &nn, 0 ); // シリアルポートに対する読み込み
    if( nn == 1 )
    {
      if( sBuf[0] == 10 || sBuf[0] == 13 ) // '\r'や'\n'を受信すると文字列を閉じる
      {
        if( i != 0 )
        {
          str[i] = '\0';
          i = 0;
          printf("%s\n",str);
        }
      }
      else
      {
        str[i] = sBuf[0];
        i++;
      }
    }
  }
}

//#include <windows.h>
//#include <stdio.h>
//#include <tchar.h>
//
//int main(void)
//{
//	HANDLE Handle;
//	DCB dcb, dcb_tmp;
//	COMMTIMEOUTS tout, tout_tmp;
//	DWORD cc;
//	char dat[3];
//	int i;
//
//	Handle = CreateFile( _T("COM3"), GENERIC_READ | GENERIC_WRITE, 0, NULL,
//		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
//	if( Handle == INVALID_HANDLE_VALUE ) {
//		printf( "Error!\n" );
//		return -1;
//	}
//
//	EscapeCommFunction( Handle, CLRDTR );
//	EscapeCommFunction( Handle, CLRRTS );
//
//	GetCommState( Handle, &dcb );
//	dcb_tmp = dcb;
//	dcb.BaudRate = CBR_9600;
//	dcb.ByteSize = 8;
//	dcb.Parity = NOPARITY;
//	dcb.StopBits = ONESTOPBIT;
//	SetCommState( Handle, &dcb );
//
//	GetCommTimeouts( Handle, &tout );
//	tout_tmp = tout;
//	tout.WriteTotalTimeoutConstant = 100;
//	tout.WriteTotalTimeoutMultiplier = 0;
//	tout.ReadTotalTimeoutConstant = 100;
//	tout.ReadTotalTimeoutMultiplier = 0;
//	tout.ReadIntervalTimeout = 10;
//	SetCommTimeouts( Handle, &tout );
//
//	for( i=0; i<1000; i++ ) {
//		ReadFile( Handle, dat, 3, &cc, NULL );
//		//if(i>15){
//			printf("x = %d  ",dat[0]);		
//			printf("y = %d  ",dat[1]);		
//			printf("z = %d\n",dat[2]);	
//		//}
//	}
//
//	PurgeComm( Handle, PURGE_RXCLEAR);
//	PurgeComm( Handle, PURGE_TXCLEAR);
//	PurgeComm( Handle, PURGE_RXABORT);
//	PurgeComm( Handle, PURGE_TXABORT);
//
//	EscapeCommFunction( Handle, CLRDTR );
//	EscapeCommFunction( Handle, CLRRTS );
//	SetCommState( Handle, &dcb_tmp );
//	SetCommTimeouts( Handle, &tout_tmp );
//
//	CloseHandle( Handle );
//	return 0;
//}