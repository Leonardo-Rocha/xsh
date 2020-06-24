#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>

#define CURSES_PIPE "/in_curses"


char *str_cat_realloc(char *destiny, const char *source)
{
	size_t destiny_length = destiny ? strlen(destiny) : 0, source_length = strlen(source);
	size_t new_length = destiny_length + source_length + 1 /* NULL */;
	char *ret = destiny ? realloc(destiny, new_length) : malloc(new_length);

	if (ret)
	{
		memcpy(ret + destiny_length, source, source_length + 1 /* NULL */);
		ret[destiny_length + source_length] = 0;
	}

	return ret;
}

void main()
{
    char in_buf[30];
    char* current_path =  str_cat_realloc(NULL, getenv("PWD"));
    char* pipe_path = str_cat_realloc(current_path, CURSES_PIPE);
    FILE *pipe = fopen(pipe_path, "r");
    
    initscr();
    while(!feof(pipe))
    {
        fgets(in_buf, 30, pipe);
        printw("%s",in_buf);
    } 
    getch();
    
    endwin();
}