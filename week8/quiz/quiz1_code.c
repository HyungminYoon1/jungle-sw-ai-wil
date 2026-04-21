
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int fibonacci(int n) {

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
}

int main(void) {
    printf("%d\r\n", fibonacci(3));
    return 0;
}