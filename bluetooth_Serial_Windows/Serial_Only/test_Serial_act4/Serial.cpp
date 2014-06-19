#include <Windows.h>
#include "Serial.h"
#include "fifo.h"

// 1つのシリアル通信に関する構造体
struct _TAG_SERIAL
{
  // 通信関係
  HANDLE handle;
  DCB dcb;

  // スレッド関係
  HANDLE thread_handle;
  DWORD thread_id;
  BOOL thread_active;
  CRITICAL_SECTION cs_recv;

  // FIFO
  fifo_t * q_recv;

  char * pname;
  char * msg;
};

/* プロトタイプ宣言 */
DWORD WINAPI serial_thread( LPVOID param );

/* 1次バッファ */
#define SERIAL_TMP_BUFSIZE 128

/**
* @brief シリアル通信の開始
*/
serial_t serial_create( char * pname, unsigned int baud )
{
  serial_t obj;
  COMMTIMEOUTS timeout;

  // インスタンスメモリの確保
  obj = ( serial_t ) malloc( sizeof( struct _TAG_SERIAL ) );
  if( obj == NULL ) return NULL;
  ZeroMemory( obj, sizeof( struct _TAG_SERIAL ) );
  obj->pname = pname;

  // COMポートのハンドルを取得
  obj->handle = CreateFile(
    pname,
    GENERIC_READ|GENERIC_WRITE,
    0,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL );

  // HandleIDが取得できなかったとき
  if( obj->handle == INVALID_HANDLE_VALUE )
  {
    free( obj );
    return NULL;
  }

  // COMポートの通信設定
  GetCommState( obj->handle, &obj->dcb );
  // ボーレートの設定
  obj->dcb.BaudRate = baud;
  // シリアル通信の設定が失敗
  if( SetCommState( obj->handle, &obj->dcb ) == FALSE )
  {
    free( obj );
    return NULL;
  }

  // COMポートのタイムアウト設定
  ZeroMemory( &timeout, sizeof( COMMTIMEOUTS ) );
  timeout.ReadIntervalTimeout = MAXDWORD;
  // タイムアウト時間の設定ができなかった
  if( SetCommTimeouts( obj->handle, &timeout ) == FALSE )
  {
    free( obj );
    return NULL;
  }

  // FIFOメモリ確保
  obj->q_recv = fifo_create();
  if( obj->q_recv == NULL )
  {
    fifo_delete( obj->q_recv );
    free( obj );
    return NULL;
  }

  // スレッド開始
  InitializeCriticalSection( &obj->cs_recv );
  obj->thread_active = TRUE;
  obj->thread_handle = CreateThread(
    NULL,
    0,
    serial_thread,
    (LPVOID *)obj,
    0,
    &obj->thread_id );

  if( obj->thread_handle == NULL )
  {
    DeleteCriticalSection( &obj->cs_recv );
    fifo_delete( obj->q_recv );
    free( obj );
    return NULL;
  }

  return obj;
}

/**
* @brief シリアル通信停止
*/
void serial_delete( serial_t obj )
{
  DWORD thread_state;

  // スレッドを停止
  obj->thread_active = FALSE;
  do
  {
    Sleep(1);
    GetExitCodeThread( obj->thread_handle, &thread_state );
  } while( thread_state == STILL_ACTIVE );
  DeleteCriticalSection( &obj->cs_recv );

  // 通信ポートを閉じる
  CloseHandle( obj->handle );

  // メモリの解放
  fifo_delete( obj->q_recv );
  free( obj );
}

/**
* @brief 通信スレッド
*/
DWORD WINAPI serial_thread( LPVOID param )
{
  serial_t obj = (serial_t ) param;
  BYTE recv_buf[SERIAL_TMP_BUFSIZE];
  DWORD recv_len;
  BOOL ret;
  BOOL recv_hold = FALSE;

  while( obj->thread_active )
  {
    // 受信処理の開始
    if( recv_hold = FALSE )
    {
      ret = ReadFile( obj->handle, recv_buf, sizeof(recv_buf), &recv_len, NULL );
      // 受け取ったデータがない
      if( ret == FALSE )
      {
        obj->msg = "ReadFile faild.\n";
        break;
      }
      if( recv_len ) recv_hold = TRUE;
    }
    else if( TryEnterCriticalSection( &obj->cs_recv ) )
    {
      recv_len -= fifo_write( obj->q_recv, recv_buf, recv_len );
      LeaveCriticalSection( &obj->cs_recv );
      if( recv_len != 0 )
        obj->msg = "q_recv is fully filld.\n";
      recv_hold= FALSE;
    }
    Sleep(1);
  }

  obj->thread_active = FALSE;
  ExitThread(TRUE);
  return 0;
}

/**
* @brief 受信する関数
*/
char serial_recv( serial_t obj, char * buf, unsigned int size )
{
  unsigned int ret;
  EnterCriticalSection( &obj->cs_recv );
  ret = fifo_read( obj->q_recv, buf, size );
  LeaveCriticalSection( &obj->cs_recv );
  return ret;
}

/**
* @brief 受信したバイト数を取得
*/
unsigned int serial_recv_length( serial_t obj )
{
  return fifo_length( obj->q_recv );
}