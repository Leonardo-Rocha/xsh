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
#include <errno.h>

#define MAX_COMMANDS 50
#define MAX_COMMAND_LENGTH 1000
#define MAX_PIPED_PROGRAMS 2

#define BUFFER_SIZE 200

typedef enum
{
  BUILTIN,
  SIMPLE,
  PIPE
} command_type;

typedef struct
{
  char *input_stream;
  char *output_stream;
  char *error_stream;
} io_stream;

typedef enum
{
  COMMAND_NOT_FOUND = -1,
  BG,
  CD,
  ECHO,
  EXIT,
  EXPORT,
  FG,
  HELP,
  HISTORY,
  JOBS,
  KILL,
  SET
} builtin_command;

char error_buffer[BUFFER_SIZE];

extern int errno;

char *builtin_commands_list[] = {"bg", "cd", "echo", "exit", "export", "fg", "help", "history", "jobs", "kill", "set", NULL};

FILE *output_redirection_file = NULL;

int exit_flag = 0;

/* Initialize ncurses with some specific options and tries to read history from ~/.history */
void init_shell();

/* Prints username@hostname: dir $ */
void print_usr_dir();

/* Returns 0 if there's a non-null input, 1 otherwise. */
int read_input(char *input_string);

/* Check if the char is valid first char for a command name. [_$ a-z A-Z] */
int is_valid_char(char first_char);

/* Returns the command_type enum. */
command_type process_input_string(char *input_string, char **parsed_args, char **parsed_args_piped);

/* 
 * Parses the input looking for the pipe delimiter '|'. 
 * Returns 0 if no pipe is found, 1 otherwise. 
 */
int parse_pipe(char *input_string, char **input_string_piped);

/* Parses the whitespaces in the input_string. Assigning the result to the parsed list of strings. */
void parse_whitespaces(char *input_string, char **parsed);

/* Returns 0 on success, -1 otherwise. */
command_type handle_builtin_commands(char **parsed_args);

/* 
 * Searches in the builtin commands list. 
 * Returns the matching command enum. 
 */
builtin_command match_builtin_command(char *input);

/* 
 * Changes the working directory and PWD environment variable. 
 * Returns zero on sucess, -1 on error and errno is set to indicate the error. 
 */
int change_dir(char *path);

void _echo(char **message);

/* Prints the shell builtin commands and its details */
void print_help();

/* Prints the last 50 commands entered. */
void print_commands_history();

// void run_foreground(const char* input_sequence[], char* exec_input, char* exec_output, char* exec_error);

// void run_background(const char* input_sequence[], char* exec_input, char* exec_output, char* exec_error);

/* Returns 0 on success, -1 on error and errno is set to indicate the error. */
void exec_system_command(char **parsed_args, io_stream file_stream);

void handle_exec_error(char *command);

void exec_system_command_piped(char **parsed_args, char **parsed_args_piped);

/* Ends ncurses window and dump history to ~/.history */
void destroy_shell();

/* 
 * Calls fopen and handle errors.
 * Returns 0 if the file was opened succesfully and -1 on error, also printing to stderr. 
 */
int handle_file_open(FILE **file_stream, const char *mode, const char *file_name);

#endif
