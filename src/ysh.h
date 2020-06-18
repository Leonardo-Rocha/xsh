#ifndef _YSH_H
#define _YSH_H

#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/history.h>

#define MAX_COMMANDS_HISTORY 50
#define MAX_COMMAND_LENGTH 1000

#define BUFFER_SIZE 200

char error_buffer[BUFFER_SIZE];

char* commands_list[] = { "bg", "cd", "echo", "exit", "export", "fg", "help", "history", "jobs", "kill", "set", NULL };

/* Initialize ncurses with some specific options and tries to read history from ~/.history */
void init_shell();

/* Prints username@hostname: dir $ */
void print_usr_dir();

int read_input(char* str);

void run_foreground(const char* input_sequence[], char* exec_input, char* exec_output, char* exec_error);

void run_background(const char* input_sequence[], char* exec_input, char* exec_output, char* exec_error);

/* End ncurses window and dump history to ~/.history */
void destroy_shell();

#endif
