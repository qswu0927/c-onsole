#ifndef TEST_H
#define TEST_H

// 测试缓冲输出CHAR_INFO
// Char.UnicodeChar 字段是 WCHAR 类型，仅支持 2 字节的 UTF-16 编码字符
// UTF-8 的中文字符至少需要 3 字节，无法用 2 字节表示
void testBufferOutput();

// 测试缓冲输出wchar_t
// 不方便设置颜色了
void testBufferOutputW();

#endif