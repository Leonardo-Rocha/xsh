#include "ysh.h"

int main()
{
	char input_string[MAX_COMMAND_LENGTH] = {0}, *parsed_args[MAX_COMMANDS];
	char *parsed_args_piped[MAX_COMMANDS];
	command_type exec_flag = 0;
	int ch;
	HIST_ENTRY *history_entry;

	init_shell(); /* Start curses mode 		  */

	while (1)
	{
		print_usr_dir();

		// TODO: can't erase first character with backspace
		ch = getch();
		// buffer verification for arrow keys and signals
		// TODO: fix keys not working
		while (ch >= 402)
		{
			switch (ch)
			{
			case KEY_UP:
				history_entry = previous_history();
				if (history_entry != NULL)
					addstr(history_entry->line);
				break;
			case KEY_DOWN:
				history_entry = next_history();
				if (history_entry != NULL)
					addstr(history_entry->line);
				break;
			default:
				break;
			}
			ch = getch();
		}

		if (read_input(input_string, ch))
			continue;

		exec_flag = process_input_string(input_string, parsed_args, parsed_args_piped);

		switch (exec_flag)
		{
		case SIMPLE:
			exec_system_command(parsed_args);
			break;
		case PIPE:
			// exec_args_piped(parsed_args, parsed_args_piped);
			break;
		default:
			break;
		}

		if (exit_flag)
			break;

		refresh();
	}

	destroy_shell();

	return 0;
}

void init_shell()
{
	initscr();							// Start curses mode
	raw();									// Line buffering disabled
	halfdelay(20);					// When we do getch(), wait for 2 seconds before return ERR value
	keypad(stdscr, TRUE);		// enables keypad to use the arrow keys to scroll on the process list
	scrollok(stdscr, TRUE); // enables scroll
	idlok(stdscr, TRUE);
	// read if there's a history in ~/.history
	read_history(NULL);
}

// TODO: tint with some colors to make it more beautiful
void print_usr_dir()
{
	char *username = getenv("USER");
	char hostname[32];
	int hostname_ret = gethostname(hostname, sizeof(hostname));
	char *cwd = getcwd(NULL, 0);
	char *home = getenv("HOME");
	int home_length = strlen(home);

	// TODO: proper error handling
	if (cwd == NULL || hostname_ret != 0)
		exit(1);

	printw("%s@%s:", username, hostname);

	// change the /home/username to ~/
	if (strncasecmp(cwd, home, home_length) == 0)
	{
		char *c = (cwd + home_length);
		printw("~%s $ ", c);
	}
	else // simply prints the dir
		printw("%s $ ", cwd);

	refresh();
}

int read_input(char *input_string, char first_char)
{
	char buf[MAX_COMMAND_LENGTH];
	if (is_valid_char(first_char))
	{
		buf[0] = first_char;
		getstr(buf + 1);
	}
	else
		getstr(buf);

	if (strlen(buf) > 0)
	{
		add_history(buf);
		strcpy(input_string, buf);
		return 0;
	}
	else
		return -1;
}

int is_valid_char(char first_char)
{
	return (first_char >= 'a' && first_char <= 'z') || (first_char >= 'A' && first_char <= 'Z') ||
				 (first_char == '$') || (first_char == '_');
}

command_type process_input_string(char *input_string, char **parsed_args, char **parsed_args_piped)
{
	char *input_string_piped[MAX_PIPED_PROGRAMS];
	int piped = 0;

	piped = parse_pipe(input_string, input_string_piped);

	if (piped)
	{
		parse_whitespaces(input_string_piped[0], parsed_args);
		parse_whitespaces(input_string_piped[1], parsed_args_piped);
	}
	else
		parse_whitespaces(input_string, parsed_args);

	if (handle_builtin_commands(parsed_args) == 0)
		return BUILTIN;
	else
		return SIMPLE + piped;
}

int parse_pipe(char *input_string, char **input_string_piped)
{
	for (int i = 0; i < MAX_PIPED_PROGRAMS; i++)
	{
		input_string_piped[i] = strsep(&input_string, "|");
		if (input_string_piped[i] == NULL)
			break;
	}

	if (input_string_piped[1] == NULL)
		return 0; // returns zero if no pipe is found.
	else
		return 1;
}

void parse_whitespaces(char *input_string, char **parsed)
{
	for (int i = 0; i < MAX_COMMANDS; i++)
	{
		parsed[i] = strsep(&input_string, " ");

		if (parsed[i] == NULL)
			break;
		if (strlen(parsed[i]) == 0)
			i--;
	}
}

command_type handle_builtin_commands(char **parsed_args)
{
	// TODO: complete commands
	builtin_command current_command = match_builtin_command(parsed_args[0]);
	int ret = 0;

	switch (current_command)
	{
	case BG:
		break;
	case CD:
		ret = change_dir(parsed_args[1]);
		break;
	case ECHO:
		_echo(parsed_args + 1);
		break;
	case EXIT:
		exit_flag = 1;
		break;
	case EXPORT:
		break;
	case FG:
		break;
	case HELP:
		print_help();
		break;
	case HISTORY:
		print_commands_history();
		break;
	case JOBS:
		break;
	case KILL:
		break;
	case SET:
		break;
	default:
		// it must check if it's a system co
		ret = SIMPLE;
		break;
	}

	if (ret == -1)
	{
		printw("%s: %s: %s\n", parsed_args[0], strerror(errno), parsed_args[1]);
		ret = 0;
	}

	return ret;
}

builtin_command match_builtin_command(char *input)
{
	int i = 0;

	for (char **command = builtin_commands_list; *command != NULL; command++, i++)
	{
		if (strcmp(input, *command) == 0)
			return i;
	}

	return COMMAND_NOT_FOUND;
}

int change_dir(char *path)
{
	char *cwd;
	int ret = chdir(path);
	if (ret == 0)
	{
		cwd = getcwd(NULL, 0);
		if (cwd == NULL)
			ret = -1;
		else
			setenv("PWD", cwd, 1);
	}

	return ret;
}

void _echo(char **message)
{
	for (char **string = message; *string != NULL; string++)
	{
		if (*string != NULL)
			printw("%s ", *string);
		else
			break;
	}
	printw("\n");
}

void print_help()
{
	char print_string[] = "\tYSH General Commands Manual\n\n"
												"builtin: bg, cd, echo, exit, export, fg, help, history, jobs, kill, set\n"
												"run: \n"
												"	builtin [-options] [args ...]\n"
												"\nFor more info about each command run helpall\n";
	if (output_redirection_file != NULL)
		fprintf(output_redirection_file, "%s", print_string);
	else
		printw("%s", print_string);
}

void print_commands_history()
{
	HIST_ENTRY *history_entry;
	// TODO: set pos to last position before printing
	for (int i = 0; i < 50; i++)
	{
		history_entry = next_history();
		if (history_entry != NULL)
			printw("%s\n", history_entry->line);
		else
			break;
	}
}

// void run_background(const char* input_sequence[], char* exec_input, char* exec_output, char* exec_error)
// {
// 	char* file = (char *) input_sequence[0];
// 	pid_t pid = fork();
// 	if(pid = 0)
// 		return;
// 	else if(pid = -1)
// 		// TODO: FORK ERROR HANDLING
// 		return;
// 	// stdout and stderr redirection
// 	if(exec_input != NULL)
// 		freopen(exec_input, "r", stdin);
// 	else
// 		freopen(BACKGROUND_IN, "r", stdin);
// 	if(exec_output != NULL)
// 		freopen(exec_output, "w", stdout);
// 	else
// 		freopen(BACKGROUND_OUT, "r", stdout);
// 	if(exec_error != NULL)
// 		reopen(exec_error, "w", stderr);
// 	else
// 		freopen(BACKGROUND_ERROR, "r", stderr);
// 	execvp(file, input_sequence);
// }

// // TODO: Add parse run
// void run_foreground(const char* input_sequence[], char* exec_input, char* exec_output, char* exec_error)
// {
// 	char* file = input_sequence[0];
// 	pid_t pid = fork();
// 	if(pid = 0)
// 		return;
// 	else if(pid = -1)
// 		// TODO: FORK ERROR HANDLING
// 		return;
// 	if(exec_input != NULL)
// 		freopen(exec_input, "r", stdin);
// 	if(exec_output != NULL)
// 		freopen(exec_output, "w", stdout);
// 	if(exec_error != NULL)
// 		freopen(exec_error, "w", stderr);
// 	execvp(file, input_sequence);
// }

void exec_system_command(char **parsed_args)
{
	// Forking a child
	pid_t pid = fork();

	if (pid == -1)
	{
		printw("\nysh: Failed to fork: %s\n", strerror(errno));
		return;
	}
	else if (pid == 0)
	{
		if (execvp(parsed_args[0], parsed_args) < 0)
		{
			if (errno = ENOENT)
				printw("ysh: command not found: %s\n", parsed_args[0]);
			else
				printw("ysh: Failed to exec command: %s: ", strerror(errno));
		}
	}
	else
	{
		// waiting for child to terminate
		wait(NULL);
	}

	return;
}

void destroy_shell()
{
	// dumps the current history to the file ~/.history
	write_history(NULL);
	endwin(); /* End curses mode		  */
}

int handle_file_open(FILE **file_stream, const char *mode, const char *file_name)
{
	if (file_stream != NULL)
	{
		*file_stream = fopen(file_name, mode);
		if (*file_stream == NULL)
		{
			snprintf(error_buffer, BUFFER_SIZE, "Could not open file \"%s\"", file_name);
			// perror(error_buffer);
			return -1;
		}
	}
	return 0;
}
