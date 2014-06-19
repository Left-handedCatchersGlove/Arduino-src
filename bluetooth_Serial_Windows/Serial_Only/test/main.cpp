#include <stdio.h>

typedef struct Point
{
  char x;
  char y;
  char z;
} P;

P point;

void view(struct Point * p)
{
  printf("%d\n",p->x);
  printf("%d\n",p->y);
  printf("%d\n",p->z);

  printf("\n%d\n",&p->x);
  printf("%d\n",&p->y);
  printf("%d\n",&p->z);

  printf("\n%d\n",(p));
  printf("%d\n",(p+1));
  printf("%d\n",(p+2));
}

int main()
{
  point.x = 1;
  point.y = 2;
  point.z = 3;

  view(&point);

  point.x = 4;
  point.y = 5;
  point.z = 6;

  view(&point);

  return 0;
}