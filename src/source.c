#include <unistd.h>
#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <readline/history.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#define CURSES_IN_SIZE 20

char curses_buffer[CURSES_IN_SIZE];

FILE *out_fd;

void *thread_safada(void *varp)
{
  int c;
  printw("we are in the thread\n");
  sleep(2);
  //fgets(curses_buffer, CURSES_IN_SIZE, out_fd);
  //printw("string: %s\n", curses_buffer);

  while (c = fgetc(stdout) != EOF)
  {
    printw("%c", c);
  }
  getch();
}

void main()
{
  FILE *fd = fopen("/dev/tty", "r+");
  //SCREEN *scr = newterm(NULL, stdout, stdin);
  initscr();
  intrflush(NULL, FALSE);
  printw("This is a ncurses print \n");
  pthread_t id = 1;
  out_fd = fopen("curses_pipe", "w+");
  dup2(STDOUT_FILENO, fileno(out_fd));

  printw("This is a nporra print \n");
  nonl();
  //pthread_create(&id, NULL, thread_safada, NULL);
  //sleep(4);
  //for (int i = 0; i < 5; i++)
    //fprintf(out_fd, "This is an stdout print \n");
  //scanf("%c", &curses_buffer[0]);

  //pthread_join(id, NULL);
  //fflush(stdout);
  //dup2(STDIN_FILENO, STDOUT_FILENO);
  getch();
  fclose(out_fd);
  /*if (pid == 0)
  {
    dup2(STDIN_FILENO, STDOUT_FILENO);
    dup2(fileno(fd), STDIN_FILENO);
    execvp(args[0], args);
  }
  else
  {
    while (fgets(curses_buffer, CURSES_IN_SIZE, STDIN_FILENO))
    {
      printw("%s", curses_buffer);
    }
  }*/
}
