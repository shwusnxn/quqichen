#include <stdio.h>

int main() {
    printf("Hello, World!\n");
    char name[100];
    
    printf("请输入你的名字（使用英文）: ");
    scanf("%s", name);
    
    printf("你输入的内容是: %s\n", name);
    return 0;
}