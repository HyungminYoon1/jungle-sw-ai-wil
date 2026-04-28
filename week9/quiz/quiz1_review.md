# 문제
1. 다음 ANSI C 프로그램에서 출력되는 내용은 무엇인가?

#include <stdio.h>

int f(int x, int *py, int **ppz)

{
  int y, z;
  **ppz += 1;
   z  = **ppz;

  *py += 2;
   y = *py;
   x += 3;

   return x + y + z;
}
  

int main()

{
   int c, *b, **a;

   c = 4;
   b = &c;
   a = &b;

   printf("%d\n", f(c, b, a));

   return 0;
}

# 답변
19

# 정답
19

// 풀이
초기 상태:
c = 4;
b = &c;
a = &b;

f(c, b, a) 호출 후 함수 안에서:
x = 4
py = &c
ppz = &b

실행 순서: **ppz += 1;
**ppz는 결국 c이므로 c = 5, z = **ppz; z = 5
*py += 2; *py도 c이므로 c = 7, y = *py; y = 7
x += 3; 를 해도 x는 값으로 전달된 별도 변수이므로 x = 7

따라서 마지막 반환: return x + y + z; // 7 + 7 + 5 = 19