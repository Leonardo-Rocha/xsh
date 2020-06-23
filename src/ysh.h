#ifndef _YSH_H
#define _YSH_H

#define _GNU_SOURCE
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

#define MAX_COMMANDS 51
#define MAX_COMMAND_LENGTH 1000
#define MAX_PIPED_PROGRAMS 2
#define MAX_COMMANDS_HISTORY 50
#define MAX_REDIRECT_ARGS 30

#define BUFFER_SIZE 200

#define OCTAL 8
#define DECIMAL 10
#define HEXADECIMAL 16

#define CTRL_C 03
#define CTRL_D 04
#define CTRL_L 12
#define CTRL_Z 26

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
  CLEAR,
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

io_stream redirection_file_stream = {NULL};

char version[] = "1.0.0";

char error_buffer[BUFFER_SIZE];

extern int errno;

extern char **environ;

char *ysh_path;

char *builtin_commands_list[] = {"bg", "cd", "clear", "echo", "exit", "export", "fg", "help", "history", "jobs", "kill", "set", NULL};

int exit_flag = 0;

int command_number = 0;

/* Initialize ncurses with some specific options and tries to read history from ~/.history */
void init_shell();

/* Config the variables MYPATH and MYPS1. */
void config_environment_variables();

/* Print prompt string using the environment variable 'MYPS1'. By default is user@hostname: cwd $ */
void print_primary_prompt_string();

/* 
 * Abbreviate $HOME with a tilde.
 * Return abbreviated cwd.
 */
char *abbreviate_home(char *cwd);

/* 
 * Get the basename of the current working directory with home abbreviated with a tilde.
 * cwd must be passed as a temporary buffer.
 * Return the basename on sucess, otherwise NULL.
 */
char *get_cwd_basename(char *cwd);

/* Sets input_string_position to 0 and set the string value to '\0'. */
void reset_input_string(char *input_string, int *input_string_position);

/* Prints the history and history_index and updates input_string and input_string position with the new history line. */
void update_input_string_with_history(int history_index, char *input_string, int *input_string_position);

/* 
 * Shift input_string right or left (if num_shifts < 0). 
 * start_position is the first position to be shifted.
 */
int shift_input_string(char *input_string, int start_position, int num_shifts);

int is_valid_input_string_position(char *string, char *position_address);

void swap_char(char *a, char *b);

/* Return 0 if there's a non-null input, -1 otherwise. */
int verify_input(char *input_string);

/* Add command to history if it's not repeated. */
void add_command_to_history(char *input_string);

/* Return the command_type enum. */
command_type process_input_string(char *input_string, char **parsed_args, char **parsed_args_piped);

/* 
 * Parse the input looking for the pipe delimiter '|'. 
 * Return 0 if no pipe is found, 1 otherwise. 
 */
int parse_pipe(char *input_string, char **input_string_piped);

/* Parse the whitespaces in the input_string, assigning the result to the parsed list of strings. */
void parse_whitespaces(char *input_string, char **parsed);

/* Parse redirect symbols (<, >, 2>) in input_string, assigning the result to the parsed list of strings*/
void parse_redirects(char *input_string, char **parsed_redirects, char **parsed_args);

/* Parse and expand prompt string special characters. */
char *parse_prompt_string_special_characters(char *string);

/* 
 * Parse and consume the \nnn.
 * Base is 8 or 16.
 * Return 0 if first char is not a digit.
 */
char parse_escape_n_base(char **escape_sequence, int base);

/* 
 * Internal function to handle parse_escape_n_base call.
 * Call parse_escape_n_base, checks the ouput and manipulates escape_sequence to rollback the pointer.
 */
char _handle_escape_n_base(char **escape_sequence, int base);

int update_arg_count(int *argc, int *new_arg_flag);

void update_IO();
/* 
 * Search in the builtin commands list. 
 * Return the matching command enum. 
 */
builtin_command match_builtin_command(char *input);

/* 
 * Change the working directory and PWD environment variable. 
 * Return zero on sucess, -1 on error and errno is set to indicate the error. 
 */
int change_dir(char *path);

void _echo(char **message);

/* Parse and convert an escape sequence to a char. */
char escape_sequence_to_char(char **escape_sequence);

/* 
 * Define or redefine an environment variable.
 * config must be 'ENV_VAR'=[$APPEND_VAR:]'NEW_VALUE'
 * where [$APPEND_VAR] can be any variable and is optional. */
void export(char **config_args);

/* Print the shell builtin commands and its details */
void print_help();

/* Print the last 50 commands entered. */
void print_commands_history();

/* Sends a signal to a pid. */
void _kill(char **parsed_args);

/* Print all environment variables. */
void _set();

// void run_foreground(const char* input_sequence[], char* exec_input, char* exec_output, char* exec_error);

// void run_background(const char* input_sequence[], char* exec_input, char* exec_output, char* exec_error);

/* 
 * Parse the input_string and exec the control sequence.
 * parsed_args and parsed_args_piped are buffers to store the parsed sequence.
 */
void exec_control_sequence(char *input_string, char **parsed_args, char **parsed_args_piped);

void exec_system_command(char **parsed_args);

/* 
 * Execute FILE, searching in the `MYPATH' environment variable if it contains no slashes, with arguments ARGV.
 * Returns 0 on success, -1 on error and errno is set to indicate the error. */
int exec_mypath(const char *file, char *const argv[]);

void exec_system_command_piped(char **parsed_args, char **parsed_args_piped);

/* End ncurses window and dump history to ~/.history */
void destroy_shell();

/* Return BUILTIN if it's a builtin command or SIMPLE if it's a system command. */
command_type handle_builtin_commands(char **parsed_args);

/* Output "command not found" or "Failed to exec command". */
void handle_exec_error(char *command);

/* 
 * Call fopen and handle errors.
 * Return 0 if the file was opened succesfully, -1 on error and errno is set to indicate the error. 
 */
int handle_file_open(FILE **file_stream, const char *mode, const char *file_name);

/* Process the redirects symbols in order to assign the correct I/0 streams*/
void handle_redirect(char **parsed_redirects);

/* 
 * Safe strcat that reallocs destiny when needed and prevents buffer overflow. 
 * Destiny may be null, in this case it works like a strcpy.
 * Return NULL on failure, otherwise string destiny. Caller must invoke free on return.
 */
char *str_cat_realloc(char *destiny, const char *source);

#endif
