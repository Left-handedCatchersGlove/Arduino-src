#ifndef _SERIAL_H_

#define _SERIAL_EX_H_

#else

#define _SERIAL_EX_H_ extern

#endif /* ifndef _SERIAL_H_ */

// データを受け取るポインタを設定
typedef struct Point
{
  char x;
  char y;
  char z;
} P;

_SERIAL_EX_H_ char dat[3];

_SERIAL_EX_H_ P point;

/**
 * @brief シリアル通信の開始
 **/
void init_serial(void);

/**
 * @brief シリアル通信により、値を取得する
 **/
void connect(struct Point * P);

/**
 * @brief スタートビットを判断する
 */
bool start_bit(char * start);

/**
 * @brief エンドビットを判断する
 */
bool end_bit(char * end);

/**
 * @brief シリアル通信の終了
 **/
void end_serial(void);