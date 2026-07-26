#include <stdio.h>

int test(int i)
{
	return i++;
}

int main()
{
	static int i;

	i = test(1);
	printf("%d", i);

	return 0;
}
