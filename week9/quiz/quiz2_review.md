# 문제
2. 다음 C 코드에서 발생하는 메모리 누수(memory leak)를 찾고, 해결 방안을 제시하시오. 

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int data;
    char *description;
} item;

item* create_item(int data, const char *desc) {
    item *new_item = (item *)malloc(sizeof(item));

    if (new_item == NULL) {
        return NULL;
    }

    new_item->data = data;
    new_item->description = (char *)malloc(strlen(desc) + 1);

    strcpy(new_item->description, desc);

    return new_item;
}
 
int main() {
    item *myItem = create_item(5, "Test Item");

    printf("Item: %d, Description: %s\n",
        myItem->data, myItem->description);

    // 다른 작업 수행

    free(myItem); // 메모리 해제

    return 0;
}

# 답변
new_item->description = (char *)malloc(strlen(desc) + 1); 에서 메모리를 할당해두고, 
main 에서는 free(myItem); 만 해서 myItem 구조체 메모리만 해제되고 description 은 메모리에 남아있게 됩니다.

description은 문자열이 아니라 문자열이 저장된 다른 메모리 주소를 들고 있는 포인터입니다.
따라서 free(myItem->description); 을 하고 이어서 free(myItem); 을 해야 메모리 누수를 막을 수 있습니다.


# 정답

메모리 누수는 `new_item->description`에 할당한 메모리를 해제하지 않아서 발생한다.

`create_item()` 함수에서 구조체 `item`을 위한 메모리를 한 번 할당하고,
```c
item *new_item = (item *)malloc(sizeof(item));
```

문자열 저장을 위해 description에도 별도로 메모리를 할당한다.
```c
new_item->description = (char *)malloc(strlen(desc) + 1);
```

하지만 main()에서는 마지막에 구조체 메모리만 해제하고 있다.
```c
free(myItem);
```

이렇게 하면 myItem 구조체 자체는 해제되지만, myItem->description이 가리키던 문자열 메모리는 해제되지 않는다. 따라서 구조체를 해제하기 전에 description을 먼저 해제해야 한다.

해결 방법:
```c
free(myItem->description);
free(myItem);
```