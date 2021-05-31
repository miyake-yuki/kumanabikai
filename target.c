#include <stdio.h>

void vuln()
{
    char buffer[128];
    printf("%p\n", buffer);
    scanf("%[^\n]", buffer); // 改行以外で入力を区切らないようにしている
}

int main()
{
    vuln();
    printf("failed!\n");
    return 0;
}

// 必要な情報
// - リターンアドレスまでのオフセット = 140
// - シェルコードが格納されるアドレス = 0xffffcde0