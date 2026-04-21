# 문제
1. 다음은 C언어를 사용해 dynamic programming의 상향식 접근법으로 피보나치 수열(1, 1, 2, 3, 5, 8, ….)의 n번째 항의 값을 리턴하는 함수이다. 내용을 채워 작성하시오. 

int fibonacci(int n) {

  // 코드 시작

  // …

  // 코드 종료

}

# 답변
    if (n < 1) {
        return 0;
    }

    if (n == 1 || n == 2) {
        return 1;
    }

    int nxt = 1;
    int cur = 1;

    for (int i = 3; i <= n; i++) {
        int tmp = cur;
        cur = nxt;
        nxt = nxt + tmp;
    }

    return nxt;

# 정답
    if (n <= 0) return 0;
    if (n <= 2) return 1;

    int dp[n + 1];
    dp[1] = 1;
    dp[2] = 1;

    for (int i = 3; i <= n; i++) {
        dp[i] = dp[i - 1] + dp[i - 2];
    }

    return dp[n];