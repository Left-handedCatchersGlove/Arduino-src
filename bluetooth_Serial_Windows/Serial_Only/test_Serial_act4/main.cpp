#include <stdio.h>
#include <Windows.h>
#include <stdlib.h>

#include "Serial.h"

int main()
{
  serial_t obj = serial_create( "COM3", 9600 );
  char buf[128], len;

  if( obj == NULL )
  {
    fprintf( stderr, "オブジェクト生成失敗\n" );
    exit(1);
  }

  while(1)
  {
    len = serial_recv( obj, buf, sizeof(buf) );
    printf("buf[0] : %d\n", buf[0]);
    printf("buf[1] : %d\n", buf[1]);
    printf("buf[2] : %d\n", buf[2]);
  }

  serial_delete( obj );
  return 0;
}