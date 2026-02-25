#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>

typedef enum
{
	RIGHT,
	LEFT,
	UP,
	DOWN
} direction_t;

volatile int interruptReceived = 0;

void handleInterrupt(int signal){interruptReceived = 1;}

int getLineWidth(char* line, int size)
{
	int length = 0;

	int i = 0;
	while(i < size)
	{
		//ESC[ found
		if(line[i+1] && line[i] == '\033' && line[i+1] == '[')
		{
			i += 2;
			//skip to final character
			while((line[i] >= '0' && line[i] <= '9') || line[i] == ';')
				i++;
			//skip final character
			i++;
		}

		length++;
		i++;
	}

	return length;
}


int hasCursorMovementCode(char* line, int size)
{
	int i = 0;
	while(i < size)
	{
		//ESC[ found
		if(line[i+1] && line[i] == '\033' && line[i+1] == '[')
		{
			i += 2;
			//skip to final character
			while((line[i] >= '0' && line[i] <= '9') || line[i] == ';')
				i++;

			if(line[i] == 'G' || line[i] == 'A')
				return 1;
		}
		i++;
	}

	return 0;
}

void getDimensions(int* rows, int* cols)
{
	struct winsize w;

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
		return;

	*cols  = w.ws_col;
	*rows = w.ws_row;
	return;

	//fallback to 256 x 64
	*cols = 256;
	*rows = 64;
	return;
}

char** getLogo(int* width, int* height)
{
	//initialize buffers
	char* buf = malloc(256);
	char* lastLine = malloc(256);
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
		//check if previous line was the last
		if(buf[0] == '\n' && hasCursorMovementCode(lastLine, strlen(lastLine)))
			break;

		//copy line to art
		strcpy(art[i], buf);

		//update width and height
		int currWidth = getLineWidth(art[i], strlen(art[i]));
		if(currWidth > *width) *width = currWidth;
		(*height)++;

		strcpy(lastLine, buf);

		i++;
	}
	free(buf);
	free(lastLine);

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
	//set keyboard interrupt handler
	signal(SIGINT, handleInterrupt);

	//move to new buffer
	printf("\033[?1049h");

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
		//check interrupt
		if(interruptReceived)
		{
			break;
		}

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

		//Frame delay
		usleep(100000);
	}

	for(int i = 0; i < 128; i++)
	{
		free(art[i]);
	}
	free(art);

	//Return to initial buffer
	printf("\033[?1049l");

	return 0;
}
