#ifndef _FIFO_H_
#define _FIFO_H_

#define FIFO_BUFSIZE 1024

// FIFO データ構造
typedef struct _TAG_FIFO
{
  unsigned char buf[FIFO_BUFSIZE];
  unsigned int size;
  unsigned int read;
  unsigned int write;
} fifo_t;

fifo_t * fifo_create( void );

void fifo_delete( fifo_t * obj );

unsigned int fifo_write(fifo_t *obj, unsigned char *buf, unsigned int size);

char fifo_read( fifo_t * obj, char * buf, unsigned int size );

unsigned int fifo_length( fifo_t * obj );

#endif