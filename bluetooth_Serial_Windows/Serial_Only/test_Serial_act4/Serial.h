#ifndef _SERIAL_H_
#define _SERIAL_H_

typedef struct _TAG_SERIAL * serial_t;

/**
 * @brief シリアル通信の開始
 */
serial_t serial_create(char * pname, unsigned int baud);

/**
 * @brief シリアル通信停止
 */
void serial_delete(serial_t obj);

/**
 * @brief 受信する関数
 */
char serial_recv(serial_t obj, char * buf, unsigned int size);

/**
 * @brief 受信したバイト数を取得
 */
unsigned int serial_recv_length(serial_t obj);

#endif