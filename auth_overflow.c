#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int check_authentication(char *passward)
{
	int auth_flag = 0;
	char passward_buffer[16];

	strcpy(passward_buffer, passward);

	if (strcmp(passward_buffer, "brillig") == 0)
	{
		auth_flag = 1;
	}
	if (strcmp(passward_buffer, "outgrade") == 0)
	{
		auth_flag = 1;
	}

	return auth_flag;
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("使用方法： %s <パスワード>\n", argv[0]);
		exit(0);
	}
	if (check_authentication(argv[1]))
	{
		printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
		printf("  アクセスを許可します。\n");
		printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	}
	else
	{
		printf("\nアクセスを拒否しました。\n");
	}
	return 0;
}