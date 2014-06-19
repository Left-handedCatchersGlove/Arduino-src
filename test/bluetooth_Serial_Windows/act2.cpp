/**
 * @brief シリアル通信プログラム（スレッドあり）
 **/
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>

void getStrComm( char str[100] );
void sub( void * );
HANDLE h;
int imax = 0;
char str[1000][100];

void main() {
  int i;
  int baudRate = 9600; // ボーレート決定
  DCB dcb;
  COMMTIMEOUTS cto;
  unsigned long dummy;

  /*-------- ファイルのクリエイト/オープン --------*/
  h = CreateFile( "COM1",      // ポート指定
                  GENERIC_READ | GENERIC_WRITE, // Read/Write
                  0,   // 共有しない
                  0,   // セキュリティ属性デフォルト
                  OPEN_EXISTING, // 既存デフォルト
                  0,   // 非同期I/Oを許す
                  0
                );

  // エラー表示
  if( h == INVALID_HANDLE_VALUE ) {
    printf("Open Error\n");
    exit(1);
  }

  // 接続メッセージ
  printf("Connect.....\n");

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

  /*------- キーボードから入力されるまで受信する --------*/
  printf("Enterキーで受信スレッド開始\n");
  getchar();
  printf("受信を開始します\n");
  _beginthread( sub, 0, &dummy ); // スレッドの開始

  /*------- キーボードから入力されるまで受信し続ける --------*/
  printf("結果の出力\n");
  getchar();

  /*------- 受信データの出力 --------*/
  printf("受信データの出力\n");
  for( i = 0; i < imax; i++ ) {
    printf("%s\n",str[i]);
  }
}

/**
 * @brief スレッドによる受信データの読み込み
 **/
void sub( void * dummy ) {
  while(1) {
    getStrComm( str[imax] );
    imax++;
  }
}

/**
 * @brief 受信データの読み込み関数
 **/
void getStrComm( char str[] ) {
  int j = 0;
  unsigned long nn;
  char sBuf[1];

  while(1) {
    ReadFile( h, sBuf, 1, &nn, 0 ); // シリアルポートに対する読み込み
    if( nn == 1 ) {
      if( sBuf[0] == 10 || sBuf[0] == 13 ) { // '\r'や'\n'を受信すると文字列閉じる
        str[j] = '\0';
        if( j != 0 ) break;
      }
      else {
        str[j] = sBuf[0];
        j++;
      }
    }
  }
}
