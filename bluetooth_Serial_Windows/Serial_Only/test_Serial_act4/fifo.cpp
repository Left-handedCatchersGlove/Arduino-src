#include <stdlib.h>
#include "fifo.h"

// インスタンス生成
fifo_t * fifo_create( void )
{
  fifo_t * obj = ( fifo_t * )malloc( sizeof( fifo_t ) );
  if( obj == NULL )
    return NULL;

  obj->read = 0;
  obj->size = FIFO_BUFSIZE;
  return obj;
}

/**
* @brief インスタンス消去
*/
void fifo_delete( fifo_t * obj )
{
  free( obj );
}


unsigned int fifo_write(fifo_t *obj, unsigned char *buf, unsigned int size)
{
  unsigned int ret = 0;
  unsigned int next = (obj->write + 1) % obj->size;

  while ( next != obj->read && ret < size ) {
    obj->buf[obj->write] = buf[ret];
    obj->write = next;
    next = (obj->write + 1) % obj->size;
    ret++;
  }

  return ret;
}

/**
* @brief データを読み込む
* @return ret : 読み込めたバイト数
*/
char fifo_read( fifo_t * obj, char * buf, unsigned int size )
{
  unsigned int ret = 0;

  while( obj->read != obj->write && ret < size )
  {
    buf[ret] = obj->buf[obj->read];
    obj->read = ( obj->read + 1 ) % obj->size;
    ret++;
  }
  return ret;
}

/**
* @brief 格納したデータ数を取得する
*/
unsigned int fifo_length( fifo_t * obj )
{
  return ( obj->size + obj->write - obj->read ) % obj->size;
}