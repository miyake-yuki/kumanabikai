#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hacking.h"

//エラーメッセージを表示して終了する関数
void fatal(char *message)
{
    char error_message[100];

    strcpy(error_message, "[!!] 致命的なエラー：");
    strncat(error_message, message, 83);
    perror(error_message);
    exit(-1);
}

//malloc()とエラー判定をセットにした関数
void *ec_malloc(unsigned int size)
{
    void *ptr;
    ptr = malloc(size);
    if (ptr == NULL)
    {
        fatal("ec_malloc内でメモリ割り当てに失敗しました");
    }
    return ptr;
}

// 生のメモリを１バイトずつ16進数表現でダンプする
void dump(const unsigned char *data_buffer, const unsigned int length)
{
    unsigned char byte;

    for (unsigned int i = 0; i < length; i++)
    {
        byte = data_buffer[i];
        // バイトを16進数表現で表示
        printf("%02x ", data_buffer[i]);

        // おそらく行末の処理(16バイトごとに表示するための処理)
        if (((i % 16) == 15) || (i == length - 1))
        {
            for (unsigned int j = 0; j < 15 - (i % 16); j++)
            {
                printf("   ");
            }
            printf("| ");

            // 行内の印字可能なバイトを表示する
            for (unsigned int j = (i - (i % 16)); j <= i; j++)
            {
                byte = data_buffer[j];

                if ((byte > 31) && (byte < 127)) // 印字可能なバイトを表示する
                {
                    printf("%c", byte);
                }
                else // 印字不可能な場合は.を表示
                {
                    printf(".");
                }
            }
            printf("\n");
        }
    }
}