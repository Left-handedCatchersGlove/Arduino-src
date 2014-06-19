#include <stdio.h>
#include <windows.h>
#include <tchar.h>

#include "Serial.h"

// 読み込むデータのサイズ
#define DATA_SIZE 1

HANDLE h;             // ハンドルの定義

/** スレッド関連の定義 **/
BOOL bReading = FALSE;
HANDLE hThread;       // スレッドの定義
DWORD ComReadThread(LPDWORD);
DWORD id;

int baudRate = 9600;  // ボーレートの設定
DCB dcb;
COMMTIMEOUTS cto;     // タイムアウト変数
DWORD cc;

/**
* @brief シリアル通信の開始
**/
void init_serial(void)
{
  // 構造体の初期化
  point.x = 0;
  point.y = 0;
  point.z = 0;

  /*-------- ファイルのクリエイト/オープン --------*/
  h = CreateFile( "COM3",         // ポート」の名前
    GENERIC_READ | GENERIC_WRITE, // Read/Write
    0,                            // 共有しない
    0,                            // セキュリティ属性デフォルト
    OPEN_EXISTING,                // 既存デフォルト
    0,                            // 非同期I/Oを許す
    0 );

  if( h == INVALID_HANDLE_VALUE ) // ハンドル番号が戻ってこないとき
  {
    printf("Open Error!\n");
    exit(1);
  }

  EscapeCommFunction( h, CLRDTR );
  EscapeCommFunction( h, CLRRTS );

  /*-------- スレッドの設定をする ------------*/
  if(bReading == FALSE)
  {
    // Threadの設定
    hThread = CreateThread( NULL, // def security
      0,                          // def stack size
      (LPTHREAD_START_ROUTINE)ComReadThread,
      NULL,                       // param to pass to thread
      0,
      &id);

    if(hThread == NULL)
    {
      CloseHandle(h);
      printf("No create Thread!");
      exit(1);
    }
  }
  bReading = TRUE;

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
}

/**
* @brief シリアル通信により、値を取得する
**/
void connect(struct Point * P)
{
  char start[2];
  char end[3];

  //ReadFile(
  //  h,   // ファイルのハンドル
  //  dat, // データのバッファ(ポインタ)
  //  1,   // 読み取り対象のバイト数
  //  &cc, // 読み取ったバイト数
  //  NULL // オーバーラップ構造体のバッファ
  //  );

  /* 自分が決めたプロトコルに当てはまらない通信は除く */
  while(1)
  {
    if( protocol_start( start ) )
    {
        ReadFile( h, &P->x, DATA_SIZE, &cc, NULL );
        ReadFile( h, &P->y, DATA_SIZE, &cc, NULL );
        ReadFile( h, &P->z, DATA_SIZE, &cc, NULL );

        if( protocol_end( end ) )
          break;
    }
  }

  //ReadFile( h, dat, DATA_SIZE, &cc, NULL );
  //ReadFile( h, (dat+1), DATA_SIZE, &cc, NULL );
  //ReadFile( h, (dat+2), DATA_SIZE, &cc, NULL );
}

bool protocol_start( char * start )
{
  ReadFile( h, start, DATA_SIZE, &cc, NULL );
  ReadFile( h, (start+1), DATA_SIZE, &cc, NULL );

  if( *start == 0 )
  {
    if( *(start) == 1 )
      return true;
    else
      return false;
  }
  else
    return false;
}

  bool protocol_end( char * end )
  {
    ReadFile( h, end, DATA_SIZE, &cc, NULL );
    ReadFile( h, (end+1), DATA_SIZE, &cc, NULL );
    ReadFile( h, (end+2), DATA_SIZE, &cc, NULL );

    if( *end == 1 )
    {
      if( *(end+1) == 1 )
      {
        if( *(end+2) == 0 )
          return true;
        else
          return false;
      }
      else
        return false;
    }
    else
      return false;
  }

  /**
  * @brief シリアル通信の終了
  **/
  void end_serial(void)
  {
    PurgeComm( h, PURGE_RXCLEAR );
    PurgeComm( h, PURGE_TXCLEAR );
    PurgeComm( h, PURGE_RXABORT );
    PurgeComm( h, PURGE_TXABORT );

    EscapeCommFunction( h, CLRDTR );
    EscapeCommFunction( h, CLRRTS );
    SetCommState( h, &dcb );
    SetCommTimeouts( h, &cto );


    /*---- スレッドを停止させる ---*/
    bReading = FALSE;
    // イベントを強制的に発行させる
    SetCommMask(h, EV_RXCHAR);
    while(true)
    {
      // Threadが終了したか確認する
      GetExitCodeThread(h, &id);
      if(id == STILL_ACTIVE)
        continue;
      else 
        break;
    }
    CloseHandle(hThread);

    CloseHandle( h );
  }