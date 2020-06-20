#ifndef _YSH_H
#define _YSH_H

#define _GNU_SOURCE
#include <unistd.h>
#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/history.h>
#include <errno.h>

#define MAX_COMMANDS 51
#define MAX_COMMAND_LENGTH 1000
#define MAX_PIPED_PROGRAMS 2
#define MAX_COMMANDS_HISTORY 50

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

typedef enum
{
  STDIN,
  STDOUT,
  STDERR
} redirections;

io_stream redirection_file_stream;

char error_buffer[BUFFER_SIZE];

extern int errno;

extern char **environ;

char *builtin_commands_list[] = {"bg", "cd", "echo", "exit", "export", "fg", "help", "history", "jobs", "kill", "set", NULL};

int exit_flag = 0;

/* Initialize ncurses with some specific options and tries to read history from ~/.history */
void init_shell();

/* Config the variables MYPATH and MYPS1. */
void config_environment_variables();

/* Print username@hostname: dir $ */
void print_usr_dir();

/* Return 0 if there's a non-null input, 1 otherwise. */
int read_input(char *input_string);

/* Check if the char is valid first char for a command name. [_$ a-z A-Z] */
int is_valid_char(char first_char);

/* Return the command_type enum. */
command_type process_input_string(char *input_string, char **parsed_args, char **parsed_args_piped);

/* 
 * Parse the input looking for the pipe delimiter '|'. 
 * Return 0 if no pipe is found, 1 otherwise. 
 */
int parse_pipe(char *input_string, char **input_string_piped);

/* Parse the whitespaces in the input_string, assigning the result to the parsed list of strings. */
void parse_whitespaces(char *input_string, char **parsed);

/* Return 0 on success, -1 otherwise. */
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

/* Print the shell builtin commands and its details */
void print_help();

/* Print the last 50 commands entered. */
void print_commands_history();

/* Print all environment variables. */
void _set();

// void run_foreground(const char* input_sequence[], char* exec_input, char* exec_output, char* exec_error);

// void run_background(const char* input_sequence[], char* exec_input, char* exec_output, char* exec_error);

void exec_system_command(char **parsed_args, io_stream file_stream);

/* 
 * Execute FILE, searching in the `MYPATH' environment variable if it contains no slashes, with arguments ARGV.
 * Returns 0 on success, -1 on error and errno is set to indicate the error. */
int exec_mypath(const char *file, char *const argv[]);

/* Output "command not found" or "Failed to exec command". */
void handle_exec_error(char *command);

void exec_system_command_piped(char **parsed_args, char **parsed_args_piped);

/* End ncurses window and dump history to ~/.history */
void destroy_shell();

/* 
 * Call fopen and handle errors.
 * Return 0 if the file was opened succesfully, -1 on error and errno is set to indicate the error. 
 */
int handle_file_open(FILE **file_stream, const char *mode, const char *file_name);

#endif
