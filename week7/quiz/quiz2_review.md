# 문제
2. 다음 소스 코드를 완성하여 55와 22가 각 줄에 순서대로 출력되게 만드세요.

#include <stdio.h>

int main()

{
    int numArr[5] = { 11, 22, 33, 44, 55 };
    int *numPtrA;
    void *ptr;

    numPtrA = &numArr[2];
    ptr = numArr;

    printf("%d\n", /** 변수는numPtrA만을 사용하세요.           **/);
    printf("%d\n", /** 변수는 ptr만을 사용하세요.             **/);

    return 0;
}

---

# 답변
*(numPtrA+2)
*(ptr+1)

---

# 정답
*(numPtrA + 2)
*((int *)ptr + 1)

// 풀이
ptr이 void * int *로 캐스팅한 뒤 써야 합니다.