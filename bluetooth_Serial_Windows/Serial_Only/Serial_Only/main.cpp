/**
* @brief �V���A���ʐM�p�v���O�����i��M�̂݁j
**/
#include <windows.h>
#include <stdio.h>
#include <tchar.h>

int main(void)
{
  HANDLE h;             // �n���h���̒�`
  int baudRate = 9600;  // �{�[���[�g�̐ݒ�
  DCB dcb;
  COMMTIMEOUTS cto;     // �^�C���A�E�g�ϐ�
  DWORD cc;

  char dat0; // �o�br�t�@�̗p��
  char dat1;
  char dat2;
  unsigned char dat[6];


  int i;       // ���[�v�ϐ�

  //h = CreateFile( _T("COM3"), 
  //                      GENERIC_READ | GENERIC_WRITE, 
  //                      0, 
  //                      NULL,
  //                      OPEN_EXISTING,
  //                      FILE_ATTRIBUTE_NORMAL,
  //                      NULL );

  /*-------- �t�@�C���̃N���G�C�g/�I�[�v�� --------*/
  h = CreateFile( "COM4",                        // �|�[�g�v�̖��O
                   GENERIC_READ | GENERIC_WRITE, // Read/Write
                   0,                            // ���L���Ȃ�
                   0,                            // �Z�L�����e�B�����f�t�H���g
                   OPEN_EXISTING,                // �����t�@�C��
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