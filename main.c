#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define ROWS 64
#define COLS 256

char** getLogo(int* width, int* height)
{
	//initialize buffers
	char* buf = malloc(256);
	char** art = malloc(128 * sizeof(char*));
	for(int i = 0; i < 128; i++)
	{
		art[i] = malloc(2048);
	}

	//get OS logo art from fastfetch
	FILE* artFile = popen("fastfetch --pipe none --structure none", "r");
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
	*width = 0;
	*height = 0;
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

		int currWidth = strlen(art[i]);
		if(currWidth > *width) *width = currWidth;
		(*height)++;

		i++;
	}
	free(buf);

	return art;
}

void printLogo(char** logo, int width, int height, unsigned int row, unsigned int col)
{
	//return if out of bounds
	if(row + height > ROWS) return;
	if(col + width > COLS) return;

	char* positionCode = malloc(16);
	for(int i = 0; i < height; i++)
	{
		//form escape code to go to desired position
		strcat(positionCode, "\033[");
		char str[8];
		snprintf(str, sizeof(str), "%d", row + i);
		strcat(positionCode, str);
		strcat(positionCode, ";");
		snprintf(str, sizeof(str), "%d", col);
		strcat(positionCode, str);
		strcat(positionCode, "H");

		//go to position
		printf("%s", positionCode);

		//print a line of art
		printf("%s", logo[i]);

	}
	free(positionCode);
}

int main()
{
	int width, height;
	char** art = getLogo(&width, &height);

//	//test print
//	for(int j = 0; j < height; j++)
//	{
//		printf("%s", art[j]);
//	}
//	printf("Width: %d\nHeight: %d\n", width, height);
	printLogo(art, width, height, 40, 200);


	for(int i = 0; i < 128; i++)
	{
		free(art[i]);
	}
	free(art);
}
