#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main()
{
	char* buf = malloc(256);
	char** art = malloc(128 * sizeof(char*));
	for(int i = 0; i < 128; i++)
	{
		art[i] = malloc(2048);
	}

	FILE* artFile = popen("fastfetch --pipe false --structure none", "r");
	if(!artFile)
	{
		puts("An error occured");
		puts("Is fastfetch installed?");
		exit(1);
	}

	int i = 0;
	while(fgets(buf, 256, artFile))
	{
		if(i >= 128) break;
		strcpy(art[i], buf);
		i++;
	}

	for(int j = 0; j < i; j++)
	{
		printf("%s", art[j]);
	}
}
