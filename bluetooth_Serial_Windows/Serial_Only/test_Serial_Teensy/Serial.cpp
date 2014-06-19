#include <stdio.h>
#include <windows.h>
#include <tchar.h>

#include "Serial.h"

// �ǂݍ��ރf�[�^�̃T�C�Y
#define DATA_SIZE 1

HANDLE h;             // �n���h���̒�`
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
  h = CreateFile( "COM3",                        // �|�[�g�v�̖��O
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
  //ReadFile(
  //  h,   // �t�@�C���̃n���h��
  //  dat, // �f�[�^�̃o�b�t�@(�|�C���^)
  //  3,   // �ǂݎ��Ώۂ̃o�C�g��
  //  &cc, // �ǂݎ�����o�C�g��
  //  NULL // �I�[�o�[���b�v�\���̂̃o�b�t�@
  //  );

  ReadFile( h, &P->x, DATA_SIZE, &cc, NULL );
  ReadFile( h, &P->y, DATA_SIZE, &cc, NULL );
  ReadFile( h, &P->z, DATA_SIZE, &cc, NULL );

  //ReadFile( h, dat, DATA_SIZE, &cc, NULL );
  //ReadFile( h, (dat+1), DATA_SIZE, &cc, NULL );
  //ReadFile( h, (dat+2), DATA_SIZE, &cc, NULL );
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

  CloseHandle( h );
}