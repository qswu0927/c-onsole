#include <stdio.h>
#include <windows.h>
#include "util.h"

int listLen(Node *head)
{
    int length = 0;
    Node *current = head;
    while (current != NULL)
    {
        length++;
        current = current->next;
    }
    return length;
}

char *listToStr(Node *head, char *con)
{
    if (head == NULL)
    {
        char *str = malloc(1);
        str[0] = '\0';
        return str;
    }

    // 第一次遍历计算所需空间
    int length = 0;
    Node *current = head;
    while (current != NULL)
    {
        length += snprintf(NULL, 0, "%d", current->data) + strlen(con);
        current = current->next;
    }

    // 分配内存
    char *str = malloc(length);
    if (str == NULL)
        return NULL;

    // 第二次遍历构建字符串
    current = head;
    int pos = 0;
    while (current != NULL)
    {
        pos += sprintf(str + pos, "%d", current->data);
        if (current->next != NULL)
        {
            pos += sprintf(str + pos, "%s", con);
        }
        current = current->next;
    }

    return str;
}
