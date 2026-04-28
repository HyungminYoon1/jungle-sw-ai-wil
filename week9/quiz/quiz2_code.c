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