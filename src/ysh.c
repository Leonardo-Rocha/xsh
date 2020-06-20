#include "ysh.h"

int main()
{
	char input_string[MAX_COMMAND_LENGTH] = {0}, *parsed_args[MAX_COMMANDS];
	char *parsed_args_piped[MAX_COMMANDS];
	command_type exec_flag = 0;
	int ch, y, x;
	HIST_ENTRY *history_entry;
	io_stream no_stream = {NULL, NULL, NULL};

	init_shell();

	while (1)
	{
		print_usr_dir();

		ch = getch();
		// buffer verification for arrow keys and signals
		// TODO: fix keys not working
		while (ch == KEY_DOWN || ch == KEY_UP || ch == '\n')
		{
			history_set_pos(history_length - 1);
			switch (ch)
			{
			case KEY_UP:
				history_entry = previous_history();
				if (history_entry != NULL)
					addstr(history_entry->line);
				else
					addstr("");
				refresh();
				break;
			case KEY_DOWN:
				history_entry = next_history();
				if (history_entry != NULL)
					addstr(history_entry->line);
				else
					addstr("");
				refresh();
				break;
			case '\n':
				print_usr_dir();
				printw("\n");
				print_usr_dir();
			default:
				break;
			}
			ch = getch();
		}
		refresh();

		ungetch(ch);
		getyx(stdscr, y, x);
		move(y, --x);

		if (read_input(input_string))
			continue;

		exec_flag = process_input_string(input_string, parsed_args, parsed_args_piped);

		switch (exec_flag)
		{
		case SIMPLE:
			exec_system_command(parsed_args, no_stream);
			break;
		case PIPE:
			exec_system_command_piped(parsed_args, parsed_args_piped);
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
	keypad(stdscr, TRUE);		// enables keypad to use the arrow keys to scroll on the process list
	scrollok(stdscr, TRUE); // enables scroll
	idlok(stdscr, TRUE);
	using_history();
	// read if there's a history in ~/.history
	read_history(NULL);
	config_environment_variables();
}

void config_environment_variables()
{
	// MYPATH
	char *env_path = getenv("PATH");
	setenv("MYPATH", env_path, 1);
	// TODO: MYPS1
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

	if (cwd == NULL || hostname_ret != 0)
	{
		printw("ysh: failed to print user info: %s", strerror(errno));
		printw("exiting...");
		destroy_shell();
		exit(EXIT_FAILURE);
	}

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

int read_input(char *input_string)
{
	char buf[MAX_COMMAND_LENGTH];
	// TODO: verify if the input is well formed
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
				 (first_char == '$') || (first_char == '_') || (first_char == '.');
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
	int i = 0;

	for (i = 0; i < MAX_COMMANDS - 1; i++)
	{
		parsed[i] = strsep(&input_string, " ");

		if (parsed[i] == NULL)
		{
			++i;
			break;
		}
		if (strlen(parsed[i]) == 0)
			i--;
	}
	parsed[i] = NULL;
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
		_set();
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
	FILE *output_file = NULL;

	if (redirection_file_stream.output_stream != NULL &&
			handle_file_open(&output_file, "w+", redirection_file_stream.output_stream) == -1)
		printw("echo: failed to redirect output to file '%s': %s", redirection_file_stream.output_stream, strerror(errno));

	// it's a env variable
	if (message[0][0] == '$')
	{
		char *env_variable = getenv(message[0] + 1);
		if (output_file == NULL)
			printw("%s\n", env_variable);
		else
			fprintf(output_file, "%s\n", env_variable);
	}
	else
	{
		// it's a string list
		for (char **string = message; *string != NULL; string++)
		{
			if (*string != NULL)
			{
				if (output_file == NULL)
					printw("%s ", *string);
				else
					fprintf(output_file, "%s ", *string);
			}
			else
			{
				break;
			}
		}
		printw("\n");
	}

	if (output_file != NULL)
		fclose(output_file);
}

void print_help()
{
	char print_string[] = "\tYSH General Commands Manual\n\n"
												"builtin: bg, cd, echo, exit, export, fg, help, history, jobs, kill, set\n"
												"run: \n"
												"	builtin [-options] [args ...]\n"
												"\nFor more info about each command run helpall\n";
	if (redirection_file_stream.output_stream == NULL)
		printw("%s", print_string);
	else
	{
		FILE *output_file = NULL;
		if (handle_file_open(&output_file, "w+", redirection_file_stream.output_stream) == 0)
		{
			fprintf(output_file, "%s", print_string);
			fclose(output_file);
		}
		else
			printw("help: failed to redirect output to file '%s': %s", redirection_file_stream.output_stream, strerror(errno));
	}
}

void print_commands_history()
{
	register HIST_ENTRY **history;
	history = history_list();
	FILE *output_file = NULL;
	int i = history_length > MAX_COMMANDS_HISTORY ? history_length - (MAX_COMMANDS_HISTORY + 1) : 0;

	if (redirection_file_stream.output_stream != NULL &&
			handle_file_open(&output_file, "w+", redirection_file_stream.output_stream) == -1)
		printw("history: failed to redirect output to file '%s': %s", redirection_file_stream.output_stream, strerror(errno));

	if (history != NULL)
	{
		for (; history[i]; i++)
		{
			if (output_file == NULL)
				printw("%s\n", history[i]->line);
			else
				fprintf(output_file, "%s\n", history[i]->line);
		}
	}

	if (output_file != NULL)
		fclose(output_file);
}

void _set()
{
	FILE *output_file = NULL;

	if (redirection_file_stream.output_stream != NULL &&
			handle_file_open(&output_file, "w+", redirection_file_stream.output_stream) == -1)
		printw("set: failed to redirect output to file '%s': %s", redirection_file_stream.output_stream, strerror(errno));

	for (char **variable = environ; *variable != NULL; variable++)
	{
		if (output_file == NULL)
			printw("%s\n", *variable);
		else
			fprintf(output_file, "%s\n", *variable);
	}

	if (output_file != NULL)
		fclose(output_file);
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

void parse_redirects(char *input_string, char **parsed_redirects, char **parsed_args)
{
	// ./media 2 3 >out < message 2> err
	// parsed_args = {"./media>, "out<in", "2>", "err"}
	int argc = 0, new_arg_flag = 0;
	char *string2separate, *separator_ret = NULL;
	for (char **arg = parsed_args; *arg != NULL; arg++)
	{
		string2separate = malloc(strlen(*arg));
		strcpy(string2separate, *arg);
		// "./media>"
		new_arg_flag = 0;
		for (char *c = string2separate; *c != '\0'; c++)
		{
			switch (*c)
			{
			case '<':
				separator_ret = strsep(&string2separate, "<");
				break;
			case '>':
				// separator_ret = "./media" ; strinf2separate = ""
				separator_ret = strsep(&string2separate, ">");
				break;
			case '2':
				if (*(c + 1) == '>')
					separator_ret = strsep(&string2separate, "2>");
				break;
			default:
				separator_ret = NULL;
				break;
				// "in <out 2> "
				// " 1   2   2
			}

			if (separator_ret != NULL)
			{
				c = string2separate;
				if (strlen(separator_ret) > 0)
				{
					parsed_redirects[argc] = separator_ret;
					if (new_arg_flag)
					{
						argc++;
					}
					else
						new_arg_flag = 1;
				}
			}
		}
		if (strlen(string2separate) > 0)
		{
			parsed_redirects[argc] = string2separate;
			if (new_arg_flag)
				argc++;
		}
		// free(string2separate);
		argc++;
	}
}

int handle_redirect(char **parsed_args, io_stream file_stream)
{
	return 0;
}

void exec_system_command(char **parsed_args, io_stream file_stream)
{
	// Forking a child
	pid_t pid = fork();

	if (pid == -1)
	{
		printw("ysh: Failed to fork: %s\n", strerror(errno));
		return;
	}
	else if (pid == 0)
	{
		//handle_redirect(parsed_args, file_stream);
		if (exec_mypath(parsed_args[0], parsed_args) < 0)
			handle_exec_error(parsed_args[0]);
	}
	else
	{
		// waiting for child to terminate
		wait(NULL);
		fflush(stdout);
	}

	return;
}

int exec_mypath(const char *file, char *const argv[])
{
	char *mypath = getenv("MYPATH");
	int ret = 0;
	if (mypath)
	{
		char mypathenv[strlen(mypath) + sizeof("MYPATH=")];
		sprintf(mypathenv, "MYPATH=%s", mypath);
		char *envp[] = {mypathenv, NULL};
		ret = execvpe(file, argv, envp);
	}
	else
	{
		printw("ysh: MYPATH env variable doesn't exist.\n");
		ret = -1;
	}

	return ret;
}

void handle_exec_error(char *command)
{
	if (errno = ENOENT)
		printw("ysh: command not found: %s\n", command);
	else
		printw("ysh: Failed to exec command: %s: ", strerror(errno));
	refresh();
	exit(EXIT_FAILURE);
}

void exec_system_command_piped(char **parsed_args, char **parsed_args_piped)
{
	int pipe_fd[2];
	pid_t p1, p2;

	if (pipe(pipe_fd) < 0)
	{
		printw("ysh: Pipe could not be initialized: %s", strerror(errno));
		return;
	}
	p1 = fork();
	if (p1 < 0)
	{
		printw("ysh: Failed to fork: %s\n", strerror(errno));
		return;
	}

	if (p1 == 0)
	{
		// It only needs to write at the write end
		close(pipe_fd[0]);
		dup2(pipe_fd[1], STDOUT_FILENO);
		close(pipe_fd[1]);

		if (exec_mypath(parsed_args[0], parsed_args) < 0)
			handle_exec_error(parsed_args[0]);
	}
	else
	{
		// Parent executing
		p2 = fork();

		if (p2 < 0)
		{
			printw("ysh: Failed to fork: %s\n", strerror(errno));
			return;
		}

		// It only needs to read at the read end
		if (p2 == 0)
		{
			close(pipe_fd[1]);
			dup2(pipe_fd[0], STDIN_FILENO);
			close(pipe_fd[0]);
			if (exec_mypath(parsed_args_piped[0], parsed_args_piped) < 0)
				handle_exec_error(parsed_args_piped[0]);
		}
		else
		{
			// parent executing, waiting for two children
			wait(NULL);
			wait(NULL);
		}
	}
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
			// snprintf(error_buffer, BUFFER_SIZE, "Could not open file \"%s\"", file_name);
			// perror(error_buffer);
			return -1;
		}
	}
	return 0;
}
