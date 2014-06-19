#include <Windows.h>
#include "Serial.h"
#include "fifo.h"

// 1�̃V���A���ʐM�Ɋւ���\����
struct _TAG_SERIAL
{
  // �ʐM�֌W
  HANDLE handle;
  DCB dcb;

  // �X���b�h�֌W
  HANDLE thread_handle;
  DWORD thread_id;
  BOOL thread_active;
  CRITICAL_SECTION cs_recv;

  // FIFO
  fifo_t * q_recv;

  char * pname;
  char * msg;
};

/* �v���g�^�C�v�錾 */
DWORD WINAPI serial_thread( LPVOID param );

/* 1���o�b�t�@ */
#define SERIAL_TMP_BUFSIZE 128

/**
* @brief �V���A���ʐM�̊J�n
*/
serial_t serial_create( char * pname, unsigned int baud )
{
  serial_t obj;
  COMMTIMEOUTS timeout;

  // �C���X�^���X�������̊m��
  obj = ( serial_t ) malloc( sizeof( struct _TAG_SERIAL ) );
  if( obj == NULL ) return NULL;
  ZeroMemory( obj, sizeof( struct _TAG_SERIAL ) );
  obj->pname = pname;

  // COM�|�[�g�̃n���h�����擾
  obj->handle = CreateFile(
    pname,
    GENERIC_READ|GENERIC_WRITE,
    0,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL );

  // HandleID���擾�ł��Ȃ������Ƃ�
  if( obj->handle == INVALID_HANDLE_VALUE )
  {
    free( obj );
    return NULL;
  }

  // COM�|�[�g�̒ʐM�ݒ�
  GetCommState( obj->handle, &obj->dcb );
  // �{�[���[�g�̐ݒ�
  obj->dcb.BaudRate = baud;
  // �V���A���ʐM�̐ݒ肪���s
  if( SetCommState( obj->handle, &obj->dcb ) == FALSE )
  {
    free( obj );
    return NULL;
  }

  // COM�|�[�g�̃^�C���A�E�g�ݒ�
  ZeroMemory( &timeout, sizeof( COMMTIMEOUTS ) );
  timeout.ReadIntervalTimeout = MAXDWORD;
  // �^�C���A�E�g���Ԃ̐ݒ肪�ł��Ȃ�����
  if( SetCommTimeouts( obj->handle, &timeout ) == FALSE )
  {
    free( obj );
    return NULL;
  }

  // FIFO�������m��
  obj->q_recv = fifo_create();
  if( obj->q_recv == NULL )
  {
    fifo_delete( obj->q_recv );
    free( obj );
    return NULL;
  }

  // �X���b�h�J�n
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
* @brief �V���A���ʐM��~
*/
void serial_delete( serial_t obj )
{
  DWORD thread_state;

  // �X���b�h���~
  obj->thread_active = FALSE;
  do
  {
    Sleep(1);
    GetExitCodeThread( obj->thread_handle, &thread_state );
  } while( thread_state == STILL_ACTIVE );
  DeleteCriticalSection( &obj->cs_recv );

  // �ʐM�|�[�g�����
  CloseHandle( obj->handle );

  // �������̉��
  fifo_delete( obj->q_recv );
  free( obj );
}

/**
* @brief �ʐM�X���b�h
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
    // ��M�����̊J�n
    if( recv_hold = FALSE )
    {
      ret = ReadFile( obj->handle, recv_buf, sizeof(recv_buf), &recv_len, NULL );
      // �󂯎�����f�[�^���Ȃ�
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
* @brief ��M����֐�
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
* @brief ��M�����o�C�g�����擾
*/
unsigned int serial_recv_length( serial_t obj )
{
  return fifo_length( obj->q_recv );
}