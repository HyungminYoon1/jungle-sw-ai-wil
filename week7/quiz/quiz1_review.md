# 문제
1. 다음 코드의 printf 출력 결과를 적으시오. 

#include <stdio.h>

int main(int argc, char *argv[])

{
    int arr[2][3] = {1,2,3,4,5,6};
    int (*p)[3] = NULL;
    p = arr;

    printf("%d\n", *(p[0] + 1) + *(p[1] + 2));
    printf("%d\n", *(*(p + 1) + 0) + *(*(p + 1) + 1));

    return 0;
}

---

# 답변
8
9

---

# 정답
8
9

풀이

*(p[0] + 1) -> arr[0][1] -> 2
*(p[1] + 2) -> arr[1][2] -> 6
첫 번째 printf -> 2 + 6 = 8

*( *(p + 1) + 0) -> arr[1][0] -> 4
*( *(p + 1) + 1) -> arr[1][1] -> 5
두 번째 printf -> 4 + 5 = 9
