#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __linux__
#include <unistd.h>
#include <sys/ioctl.h>
#endif

typedef enum
{
	RIGHT,
	LEFT,
	UP,
	DOWN
} direction_t;

void getDimensions(int* rows, int* cols)
{
	#ifdef _WIN32
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

		if (!GetConsoleScreenBufferInfo(h, &csbi))
			return 0;

		*width  = csbi.srWindow.Right  - csbi.srWindow.Left + 1;
		*height = csbi.srWindow.Bottom - csbi.srWindow.Top  + 1;
		return;
	#endif

	#ifdef __linux__
		struct winsize w;

		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
			return;

		*cols  = w.ws_col;
		*rows = w.ws_row;
		return;
	#endif

	//fallback to 256 x 64
	*cols = 256;
	*rows = 64;
	return;
}

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
		//regmatch_t match;

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

void printLogo(char** logo, int width, int height, int termRows, int termCols, unsigned int row, unsigned int col)
{
	//return if out of bounds
	if(row + height > termRows) return;
	if(col + width > termCols) return;

	char* positionCode = malloc(16);
	*positionCode = 0;
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

void updateDirection(direction_t* dir, int* currRow, int* currCol, int artWidth, int artHeight, int terminalRows, int terminalCols)
{
	if(*currRow <= 1) dir[0] = DOWN;
	if(*currRow + artHeight >= terminalRows) dir[0] = UP;
	if(*currCol <= 0) dir[1] = RIGHT;
	if(*currCol + artWidth >= terminalCols) dir[1] = LEFT;


	switch(dir[0])
	{
		case UP:
			(*currRow)--;
			break;
		case DOWN:
			(*currRow)++;
			break;
	}
	switch(dir[1])
	{
		case LEFT:
			(*currCol)-=2;
			break;
		case RIGHT:
			(*currCol)+=2;
			break;
	}
}

int main()
{
	//get logo art
	int artWidth, artHeight;
	char** art = getLogo(&artWidth, &artHeight);

	//initialize position and direction
	int currRow = 0, currCol = 0;
	direction_t dir[2] = {DOWN, RIGHT};

	//hide cursor
	printf("\033[25l");

	//animation loop
	while(1)
	{
		//clear screen
		printf("\033[2J");

		//get dimensions of terminal window
		int terminalRows, terminalCols;
		getDimensions(&terminalRows, &terminalCols);

		//update direction
		updateDirection(
				dir,
				&currRow, &currCol,
				artWidth, artHeight,
				terminalRows, terminalCols
				);

		//draw logo
		printLogo(
				art,
				artWidth, artHeight,
				terminalRows, terminalCols,
				currRow, currCol
			 );

		//TEST
		usleep(100000);
	}

	for(int i = 0; i < 128; i++)
	{
		free(art[i]);
	}
	free(art);
}
