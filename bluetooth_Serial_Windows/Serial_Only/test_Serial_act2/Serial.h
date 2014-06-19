#ifndef _SERIAL_H_

#define _SERIAL_EX_H_

#else

#define _SERIAL_EX_H_ extern

#endif /* ifndef _SERIAL_H_ */

// �f�[�^���󂯎��|�C���^��ݒ�
typedef struct Point
{
  char x;
  char y;
  char z;
} P;

_SERIAL_EX_H_ char dat[3];

_SERIAL_EX_H_ P point;

bool protocol_start( char * start );

bool protocol_end( char * end );

/**
 * @brief �V���A���ʐM�̊J�n
 **/
void init_serial(void);

/**
 * @brief �V���A���ʐM�ɂ��A�l���擾����
 **/
void connect(struct Point * P);

/**
 * @brief �V���A���ʐM�̏I��
 **/
void end_serial(void);