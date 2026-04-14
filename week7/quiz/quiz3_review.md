# 문제
3. 아래 코드의 실행 결과를 적어 보시오. 

#include <stdio.h>

int main() {

    char* str[2];
    str[0] = "hello!";
    str[1] = "jungler";

    printf("1. %s\n", str[0] + 1);
    printf("2. %s\n", (str + 1)[0] + 2);

    return 0;
}

---

# 답변
1. ello!
2. ngler

---

# 정답
1. ello!
2. ngler

// 풀이
str[0] + 1 -> "hello!"의 두 번째 문자부터 -> ello!
(str + 1)[0] -> str[1] -> "jungler"
"jungler" + 2 -> 세 번째 문자부터 -> ngler
