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
