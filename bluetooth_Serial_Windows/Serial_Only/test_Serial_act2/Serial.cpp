#include <stdio.h>
#include <windows.h>
#include <tchar.h>

#include "Serial.h"

// �ǂݍ��ރf�[�^�̃T�C�Y
#define DATA_SIZE 1

HANDLE h;             // �n���h���̒�`

/** �X���b�h�֘A�̒�` **/
BOOL bReading = FALSE;
HANDLE hThread;       // �X���b�h�̒�`
DWORD ComReadThread(LPDWORD);
DWORD id;

int baudRate = 9600;  // �{�[���[�g�̐ݒ�
DCB dcb;
COMMTIMEOUTS cto;     // �^�C���A�E�g�ϐ�
DWORD cc;

/**
* @brief �V���A���ʐM�̊J�n
**/
void init_serial(void)
{
  // �\���̂̏�����
  point.x = 0;
  point.y = 0;
  point.z = 0;

  /*-------- �t�@�C���̃N���G�C�g/�I�[�v�� --------*/
  h = CreateFile( "COM3",         // �|�[�g�v�̖��O
    GENERIC_READ | GENERIC_WRITE, // Read/Write
    0,                            // ���L���Ȃ�
    0,                            // �Z�L�����e�B�����f�t�H���g
    OPEN_EXISTING,                // �����f�t�H���g
    0,                            // �񓯊�I/O������
    0 );

  if( h == INVALID_HANDLE_VALUE ) // �n���h���ԍ����߂��Ă��Ȃ��Ƃ�
  {
    printf("Open Error!\n");
    exit(1);
  }

  EscapeCommFunction( h, CLRDTR );
  EscapeCommFunction( h, CLRRTS );

  /*-------- �X���b�h�̐ݒ������ ------------*/
  if(bReading == FALSE)
  {
    // Thread�̐ݒ�
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

  /*-------- �V���A���|�[�g�̏�ԑ��� --------*/
  GetCommState( h, &dcb ); // �V���A���|�[�g�̏�Ԃ��擾
  dcb.BaudRate = baudRate;
  SetCommState( h, &dcb ); // �V���A���|�[�g�̏�Ԃ�ݒ�

  /*-------- �V���A���|�[�g�̃^�C���A�E�g��ԑ��� --------*/
  GetCommTimeouts( h, &cto ); // �^�C���A�E�g�ݒ��Ԃ��擾
  cto.ReadIntervalTimeout = 1000;
  cto.ReadTotalTimeoutMultiplier = 0;
  cto.ReadTotalTimeoutConstant = 1000;
  cto.WriteTotalTimeoutMultiplier = 0;
  cto.WriteTotalTimeoutConstant = 0;
  SetCommTimeouts( h, &cto ); // �^�C���A�E�g��Ԃ�ݒ�
}

/**
* @brief �V���A���ʐM�ɂ��A�l���擾����
**/
void connect(struct Point * P)
{
  char start[2];
  char end[3];

  //ReadFile(
  //  h,   // �t�@�C���̃n���h��
  //  dat, // �f�[�^�̃o�b�t�@(�|�C���^)
  //  1,   // �ǂݎ��Ώۂ̃o�C�g��
  //  &cc, // �ǂݎ�����o�C�g��
  //  NULL // �I�[�o�[���b�v�\���̂̃o�b�t�@
  //  );

  /* ���������߂��v���g�R���ɓ��Ă͂܂�Ȃ��ʐM�͏��� */
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
  * @brief �V���A���ʐM�̏I��
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


    /*---- �X���b�h���~������ ---*/
    bReading = FALSE;
    // �C�x���g�������I�ɔ��s������
    SetCommMask(h, EV_RXCHAR);
    while(true)
    {
      // Thread���I���������m�F����
      GetExitCodeThread(h, &id);
      if(id == STILL_ACTIVE)
        continue;
      else 
        break;
    }
    CloseHandle(hThread);

    CloseHandle( h );
  }