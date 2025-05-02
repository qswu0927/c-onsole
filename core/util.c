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

FILE *safeFopen(const char *filename, const char *mode)
{
    FILE *file = NULL;
    errno_t err = fopen_s(&file, filename, mode);
    if (err != 0)
    {
        perror("Failed to open file");
        return NULL;
    }
    return file;
}

void loadConfig(const char *filename, void *config, size_t size)
{
    FILE *file = safeFopen(filename, "rb");
    if (!file)
        return;
    fread(config, size, 1, file);
    fclose(file);
}

void saveConfig(const char *filename, void *config, size_t size)
{
    FILE *file = safeFopen(filename, "wb");
    fwrite(config, size, 1, file);
    fclose(file);
}

/**
 * 随机打乱数组（洗牌）
 */
void shuffle(int arr[], int n)
{
    for (int i = n - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

/**
 * 矩阵顺时针旋转90度
 */
void rotate90Clockwise(int rows, int cols, CHAR **mat, CHAR **result)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            result[j][rows - 1 - i] = mat[i][j];
        }
    }
}
