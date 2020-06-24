#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define CURSES_PIPE "/in_cursed"
#define CONSUMER "/curses_out"

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
    
    char* current_path =  str_cat_realloc(NULL, getenv("PWD"));
    char* pipe_path =  str_cat_realloc(NULL, current_path);
    pipe_path = str_cat_realloc(NULL, CURSES_PIPE);
    FILE *pipe = fopen(pipe_path, "w+");
    printf("OH DESGRAÇA EU TO AQUI");
    pid_t pid_branch = fork();
    if(pid_branch = 0)
    {
        char* arg_list[] = {NULL};
        char*  curses_proc = str_cat_realloc(current_path, CURSES_PIPE);
        execvp(curses_proc, arg_list);
    }
    for(int i = 0; i < 2000; i++)
        fprintf(pipe,"teste teste2\n teste3 teste4\n teste5 \n");
    
    wait((int*)-1);
}
