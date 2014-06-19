/**
* @brief シリアル通信用プログラム（受信のみ）
**/
#include <windows.h>
#include <stdio.h>
#include <tchar.h>

int main(void)
{
  HANDLE h;             // ハンドルの定義
  int baudRate = 9600;  // ボーレートの設定
  DCB dcb;
  COMMTIMEOUTS cto;     // タイムアウト変数
  DWORD cc;

  char dat0; // バッrファの用意
  char dat1;
  char dat2;
  unsigned char dat[6];


  int i;       // ループ変数

  //h = CreateFile( _T("COM3"), 
  //                      GENERIC_READ | GENERIC_WRITE, 
  //                      0, 
  //                      NULL,
  //                      OPEN_EXISTING,
  //                      FILE_ATTRIBUTE_NORMAL,
  //                      NULL );

  /*-------- ファイルのクリエイト/オープン --------*/
  h = CreateFile( "COM4",                        // ポート」の名前
                   GENERIC_READ | GENERIC_WRITE, // Read/Write
                   0,                            // 共有しない
                   0,                            // セキュリティ属性デフォルト
                   OPEN_EXISTING,                // 既存ファイル
                   0,                            // 非同期I/Oを許す
                   0 );

  if( h == INVALID_HANDLE_VALUE ) // ハンドル番号が戻ってこないとき
  {
    printf("Open Error!\n");
    exit(1);
  }

  EscapeCommFunction( h, CLRDTR );
  EscapeCommFunction( h, CLRRTS );

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

  while(1)
  {
    ReadFile( h, &dat[0], 1, &cc, NULL );
    ReadFile( h, &dat[1], 1, &cc, NULL );
    //ReadFile( h, &dat[2], 1, &cc, NULL );

    printf("x = %d  \n",dat[0]);
    printf("y = %d  \n\n",dat[1]);
    //printf("z = %d  \n\n",dat[2]);
  }

  PurgeComm( h, PURGE_RXCLEAR );
  PurgeComm( h, PURGE_TXCLEAR );
  PurgeComm( h, PURGE_RXABORT );
  PurgeComm( h, PURGE_TXABORT );

  EscapeCommFunction( h, CLRDTR );
  EscapeCommFunction( h, CLRRTS );
  SetCommState( h, &dcb );
  SetCommTimeouts( h, &cto );

  CloseHandle( h );
  return 0;
}