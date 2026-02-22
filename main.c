#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main()
{
	if(system("fastfetch --pipe false > artFile") != 0)
	{
		puts("An error occured");
		puts("Is fastfetch installed?");
		exit(1);
	}
}
