#include <stdio.h>

int main()
{
	char text[] = "/root", *mark=NULL,*p = text;

	printf("当前目录: [%s]\n", p);

	// 获取上级目录
	while(*p != '\0')
	{
		if (*p == '/')
		{
			mark = p;
		}
		p++;
	}
	*(mark+1) = '\0';
	printf("[%c]\n", '/');
	printf("%s\n", text);
}
