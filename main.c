#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
int main()
{
	//initialize buffers
	char* buf = malloc(256);
	char** art = malloc(128 * sizeof(char*));
	char** plainArt = malloc(128 * sizeof(char*));
	for(int i = 0; i < 128; i++)
	{
		art[i] = malloc(2048);
		plainArt[i] = malloc(2048);
	}

	//get OS logo art from fastfetch
	FILE* artFile = popen("fastfetch --structure none", "r");
	if(!artFile)
	{
		puts("An error occured");
		puts("Is fastfetch installed?");
		exit(1);
	}

	//Store logo in memory while removing (non-coloring) escape codes
	regex_t regex;
	regcomp(&regex, "\033\\[(?!([0-9;]*m))([0-9;?]*[A-Za-z])", REG_EXTENDED);

	//calculate width and height while storing logo
	int width = 0, height = 0;
	int i = 0;

	//read line by line
	while(fgets(buf, 256, artFile))
	{
		if(i >= 128) break;
		if(strcmp(buf, "\n") == 0) break;

		char* src = buf;
		char* dest = art[i];
		regmatch_t match;

		//remove non-color ascii escape codes
		while(*src)
		{
			if(*src == '\033' && src[1] == '[')
			{
				// Check if this is a color (ends with 'm')
				char* seq = src + 2;
				int isColor = 0;
				while(*seq && *seq != 'm' && ((*seq >= '0' && *seq <= '9') || *seq == ';')) seq++;
				if(*seq == 'm') isColor = 1;

				if(isColor)
				{
					// Copy the entire color sequence
					seq++; // include 'm'
					while(src < seq) *dest++ = *src++;
				}
				else
				{
					// Skip the escape sequence
					while(*src && !((*src >= 'A' && *src <= 'Z') || (*src >= 'a' && *src <= 'z'))) src++;
					if(*src) src++; // skip the command char
				}
			}
			else
			{
				*dest++ = *src++;
			}
		}
		*dest = '\0';
		printf("Got a line:\t%s\n", art[i]);
		printf("Height: %d\n", height);

		int currWidth = strlen(art[i]);
		printf("Currwidth: %d\n", currWidth);
		if(currWidth > width) width = currWidth;
		height++;

		i++;
	}


	for(int j = 0; j < i; j++)
	{
		printf("%s", art[j]);
	}
	printf("Width: %d\nHeight: %d\n", width, height);
}
