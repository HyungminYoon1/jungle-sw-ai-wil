// 1. 다음 ANSI C 프로그램에서 출력되는 내용은 무엇인가?

#include <stdio.h>

int f(int x, int *py, int **ppz)

{
  int y, z;
  **ppz += 1;
   z  = **ppz;

  *py += 2;
   y = *py;
   x += 3;

   printf("x == %d\n", x); 
   printf("y == %d\n", y); 
   printf("z == %d\n", z); 

   return x + y + z; // 22
}

// x = 7, y = 7, z = 5

int main()

{
   int c, *b, **a;

   c = 4;
   b = &c;
   a = &b;

   printf("%d\n", f(c, b, a));   

   return 0; 
}

