#define _SERIAL_H_
#include <stdio.h>
#include "Serial.h"
#include <math.h>

void printPosition(void);

void main()
{
  char pre_dat = 0;

  init_serial();

  printf("--------------------------------\n");

  while(1)
  {
    connect(&point);
    printPosition();
    printf("------------------\n");
  }
}

void printPosition(void)
{
  printf("x : %d\n",point.x);
  printf("y : %d\n",point.y);
  printf("z : %d\n",point.z);
}