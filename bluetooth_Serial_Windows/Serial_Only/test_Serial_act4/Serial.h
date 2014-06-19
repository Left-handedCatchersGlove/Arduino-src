#ifndef _SERIAL_H_
#define _SERIAL_H_

typedef struct _TAG_SERIAL * serial_t;

/**
 * @brief �V���A���ʐM�̊J�n
 */
serial_t serial_create(char * pname, unsigned int baud);

/**
 * @brief �V���A���ʐM��~
 */
void serial_delete(serial_t obj);

/**
 * @brief ��M����֐�
 */
char serial_recv(serial_t obj, char * buf, unsigned int size);

/**
 * @brief ��M�����o�C�g�����擾
 */
unsigned int serial_recv_length(serial_t obj);

#endif