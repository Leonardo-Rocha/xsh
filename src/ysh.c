#include "ysh.h"

int main(int argc, char const *argv[])
{
	char input_string[MAX_COMMAND_LENGTH] = {0}, *parsed_args[MAX_COMMANDS];
	char *parsed_args_piped[MAX_COMMANDS];
	command_type exec_flag = 0;
	int ch, y, x;
	HIST_ENTRY *history_entry;
	io_stream no_stream = {NULL, NULL, NULL};

	init_shell(argv[0]);

	while (1)
	{
		print_primary_prompt_string();

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
				print_primary_prompt_string();
				printw("\n");
				print_primary_prompt_string();
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

void init_shell(const char *_ysh_path)
{
	initscr();							// Start curses mode
	raw();									// Line buffering disabled
	keypad(stdscr, TRUE);		// enables keypad to use the arrow keys to scroll on the process list
	scrollok(stdscr, TRUE); // enables scroll
	idlok(stdscr, TRUE);
	using_history();
	// read if there's a history in ~/.history
	read_history(NULL);
	config_environment_variables(ysh_path);
}

void config_environment_variables(const char *_ysh_path)
{
	// $MYPATH
	char *env_path = getenv("PATH");
	setenv("MYPATH", env_path, 1);
	// $MYPS1
	char *default_myps1 = "\\u@\\h:\\w \\$ ";
	if (getenv("MYPS1") == NULL)
		setenv("MYPS1", default_myps1, 1);

	// TODO: FIX THIS
	// $0 - shell path
	// ysh_path = malloc(strlen(_ysh_path) * sizeof(char));
	// strcpy(ysh_path, _ysh_path);
	// // remove "./"
	// strsep(&ysh_path, "./");
	// char *_pwd = getenv("PWD");
	// char *pwd = malloc(strlen(_pwd));
	// strcpy(pwd, _pwd);
	// // concat to get the shell absolute path
	// strcat(pwd, ysh_path);
	// // TODO: does this fuck with ysh_path?
	// ysh_path = pwd;
	// setenv("0", ysh_path, 1);
}

void print_primary_prompt_string()
{
	// TODO: tint with some colors to make it more beautiful
	// TODO2: fix being able to erase the last character
	char *_prompt_setting = getenv("MYPS1");
	char *start_address, *prompt_setting = malloc(strlen(_prompt_setting));

	start_address = prompt_setting;

	strcpy(prompt_setting, _prompt_setting);

	for (char *c = prompt_setting; *c != '\0'; c++)
	{
		if (*c != '\\')
			printw("%c", *c);
		else
			c = parse_prompt_string_special_characters(c);
	}

	free(start_address);
}

char *parse_prompt_string_special_characters(char *string)
{
	// TODO: complete cases
	char *buffer = malloc(BUFFER_SIZE * sizeof(char));
	char *buffer_start_address = buffer;
	int ret;
	struct stat *stat_buf = NULL;

	time_t rawtime;
	time(&rawtime);
	struct tm *timeinfo = localtime(&rawtime);

	switch (string[1])
	{
	// \a : an ASCII bell character (07)
	case 'a':
		printf("\a");
		string++;
		break;
	// \d : the date in “Weekday Month Date” format (e.g., “Tue May 26”)
	case 'd':
		strftime(buffer, BUFFER_SIZE, "%a %b %d", timeinfo);
		printw("%s", buffer);
		string++;
		break;
	// \D{format} : the format is passed to strftime(3) and the result is inserted into the prompt string;
	// an empty format results in a locale-specific time representation. The braces are required.
	case 'D':
		if (string[2] == '{')
		{
			string += 3;
			strftime(buffer, BUFFER_SIZE, strsep(&string, "}"), timeinfo);
			printw("%s", buffer);
			string--;
		}
		else
			printw("\nysh: MYPS1 syntax error: '\\D{format}'");
		break;
	// \e : an ASCII escape character (027)
	case 'e':
		// TODO: what should we do here?
		printw("\e");
		string++;
		break;
	// \h : the hostname up to the first ‘.’
	case 'h':
		ret = gethostname(buffer, BUFFER_SIZE);
		if (ret == 0)
			printw("%s", strsep(&buffer, "."));
		string++;
		break;
	// \H : the hostname
	case 'H':
		ret = gethostname(buffer, BUFFER_SIZE);
		if (ret == 0)
			printw("%s", buffer);
		string++;
		break;
	// \j : the number of jobs currently managed by the shell
	case 'j':
		// TODO: add this when jobs are done.
		break;
	// \l : the basename of the shell'ss terminal device name
	case 'l':
		// TODO: change this from "/bin/bash" to actual ysh absolute path
		stat("/bin/bash", stat_buf);
		if (stat_buf != NULL)
			printf("%d", minor(stat_buf->st_dev));
		string++;
		break;
	// \n : newline
	case 'n':
		printw("\n");
		string++;
		break;
	// \r : carriage return
	case 'r':
		printw("\r");
		string++;
		break;
	// \s : the name of the shell, the basename of $0 (the portion following the final slash)
	case 's':
		printw("ysh");
		string++;
		break;
	// \t : the current time in 24-hour HH:MM:SS format
	case 't':
		strftime(buffer, BUFFER_SIZE, "%T", timeinfo);
		printw("%s", buffer);
		string++;
		break;
	// \T : the current time in 12-hour HH:MM:SS format
	case 'T':
		strftime(buffer, BUFFER_SIZE, "%r", timeinfo);
		printw("%s", buffer);
		string++;
		break;
	// \@ : the current time in 12-hour am/pm format
	case '@':
		strftime(buffer, BUFFER_SIZE, "%r", timeinfo);
		char *hours = strsep(&buffer, ":");
		char *minutes = strsep(&buffer, ":");
		printw("%s:%s", hours, minutes);
		string++;
		break;
	// \A : the time, in 24-hour HH:MM format.
	case 'A':
		strftime(buffer, BUFFER_SIZE, "%R", timeinfo);
		printw("%s", buffer);
		string++;
		break;
	// \u : the username of the current user.
	case 'u':
		buffer = getenv("USER");
		printw("%s", buffer);
		string++;
		break;
	// \v : the version of Bash (e.g., 2.00)
	case 'v':
		// TODO: print version
		break;
	// \V : the release of Bash, version + patchlevel (e.g., 2.00.0)
	case 'V':
		// TODO: print version + patchlevel
		break;
	// \w : the current working directory, with $HOME abbreviated with a tilde (uses the $PROMPT_DIRTRIM variable).
	case 'w':
		buffer = getcwd(buffer, BUFFER_SIZE);
		buffer = abbreviate_home(buffer);
		printw("%s", buffer);
		string++;
		break;
	case 'W':
		// TODO: deal with pwd basename
		string++;
		break;
	case '$':
		printw("$");
		string++;
		break;
	default:
		break;
	}

	free(buffer_start_address);
	return string;
}

char *abbreviate_home(char *cwd)
{
	char *home = getenv("HOME");
	int home_length = strlen(home);

	if (strncasecmp(cwd, home, home_length) == 0)
	{
		char *c = (cwd + home_length);
		snprintf(cwd, strlen(cwd), "~%s", c);
	}

	return cwd;
}

int read_input(char *input_string)
{
	char buf[MAX_COMMAND_LENGTH];
	// HIST_ENTRY *history_entry;
	getstr(buf);

	if (strlen(buf) > 0)
	{
		// TODO: do not add buf if it's equal to the previous entry
		// history_entry = previous_history();
		// if (strcmp(buf, history_entry->line) != 0)
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
	// char **parsed_redirects = (char*) malloc(MAX_COMMANDS);

	piped = parse_pipe(input_string, input_string_piped);

	if (piped)
	{
		parse_whitespaces(input_string_piped[0], parsed_args);
		// parse_redirects(input_string_piped[0], parse_redirects, parsed_args);
		// parsed_args = parsed_redirects;
		
		parse_whitespaces(input_string_piped[1], parsed_args_piped);
		// parse_redirects(input_string_piped[1], parse_redirects, parsed_args_piped);
		// parsed_args_piped = parsed_redirects;
	}
	else
	{
		parse_whitespaces(input_string, parsed_args);
		// parse_redirects(input_string, parse_redirects, parsed_args);
		// parsed_args = parsed_redirects;
	}
	if (parsed_args[0] != NULL && handle_builtin_commands(parsed_args) == 0)
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
	char double_quote = '"';
	char *double_quote_position = NULL;
	char *whitespace_position = NULL;
	char *parsed_quotes = NULL;

	for (i = 0; i < MAX_COMMANDS - 1; i++)
	{
		if (input_string)
		{
			double_quote_position = strchr(input_string, double_quote);
			whitespace_position = strchr(input_string, ' ');
		}
		else
		{
			parsed[i] = NULL;
			i++;
			break;
		}

		// whitespace is after or inside double quotes
		if ((double_quote_position && whitespace_position && (double_quote_position < whitespace_position)) ||
				(double_quote_position && whitespace_position == NULL))
		{
			// store what's before first double_quote
			parsed_quotes = strsep(&input_string, &double_quote);
			if (parsed_quotes && strlen(parsed_quotes) > 0)
				parsed[i++] = parsed_quotes;
			// store what's inside double_quotes
			parsed[i] = strsep(&input_string, &double_quote);
			if (input_string == NULL)
			{
				printw("ysh: syntax error: missing closing '\"'\n");
				parsed[0] = NULL;
				return;
			}
		}
		else
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
		export(parsed_args + 1);
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
		// it must check if it's a system command
		ret = SIMPLE;
		break;
	}

	if (ret == -1)
	{
		printw("%s: %s: %s\n", parsed_args[0], strerror(errno), parsed_args[1]);
		ret = BUILTIN;
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
	// TODO: deal with \n and special characters
	FILE *output_file = NULL;

	if (redirection_file_stream.output_stream != NULL &&
			handle_file_open(&output_file, "w+", redirection_file_stream.output_stream) == -1)
		printw("echo: failed to redirect output to file '%s': %s", redirection_file_stream.output_stream, strerror(errno));

	// it's a env variable
	if (message[0] && message[0][0] == '$')
	{
		// +1 to skip '$'
		char *env_variable = getenv(message[0] + 1);
		if (env_variable == NULL)
			printw("echo: environment variable '%s' doesn't exist\n", message[0] + 1);
		else if (output_file == NULL)
			printw("%s\n", env_variable);
		else
			fprintf(output_file, "%s\n", env_variable);
	}
	else if (message[0])
	{
		// it's a string list
		for (char **string = message; *string != NULL; string++)
		{
			if (output_file == NULL)
				printw("%s ", *string);
			else
				fprintf(output_file, "%s ", *string);
		}
		printw("\n");
	}
	else
		printw("\n");

	if (output_file != NULL)
		fclose(output_file);
}

void export(char **config)
{
	char *env_variable, *append_env_variable, *append_env_variable_value;
	char *aux_config_start, *aux_config;
	int ret = 0;

	if (config[0] == NULL)
	{
		printw("Usage: export 'ENV_VAR'=[$APPEND_VAR:]'NEW_VALUE'\n");
		return;
	}

	// this must be done because strsep changes the aux_config pointer.
	aux_config_start = aux_config = malloc(strlen(config[0]));

	strcpy(aux_config, config[0]);
	// aux_config now contains the new value
	env_variable = strsep(&aux_config, "=");

	if (getenv(env_variable) == NULL)
		printw("export: environment variable '%s' doesn't exist\n", env_variable);

	if (aux_config != NULL && aux_config[0] == '$')
	{
		append_env_variable = strsep(&aux_config, ":");
		// + 1 to skip '$'
		append_env_variable_value = getenv(append_env_variable + 1);
		if (append_env_variable_value != NULL)
		{
			if (aux_config != NULL)
			{
				strcat(append_env_variable_value, ":");
				strcat(append_env_variable_value, aux_config);
			}
			ret = setenv(env_variable, append_env_variable_value, 1);
		}
		else
			printw("export: environment variable '%s' doesn't exist\n", (append_env_variable + 1));
	}
	else if (aux_config == NULL)
		printw("export: syntax error: missing '='\n");
	else if (strlen(aux_config) > 0)
		ret = setenv(env_variable, aux_config, 1);
	else if (config[1])
		ret = setenv(env_variable, config[1], 1);
	else
		ret = setenv(env_variable, "", 1);

	if (ret == -1)
	{
		printw("export: setenv error: %s", strerror(errno));
	}

	if (aux_config_start != NULL)
		free(aux_config_start);
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
				printw("%d %s\n", i, history[i]->line);
			else
				fprintf(output_file, "%d %s\n", i, history[i]->line);
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
	int argc = 0, new_arg_flag = 0;
	char *string2separate, *separator_ret = NULL, *redirect_sign = NULL;
	for (char **arg = parsed_args; *arg != NULL; arg++)
	{
		printw("arg: %s\n", *arg);
		string2separate = malloc(strlen(*arg));
		strcpy(string2separate, *arg);
		new_arg_flag = 0;
		for (char *c = string2separate; *c != '\0'; c++)
		{
			switch (*c)
			{
			case '<':
				separator_ret = strsep(&string2separate, "<");
				break;
			case '>':
				parsed_redirects[argc] = ">";
				separator_ret = strsep(&string2separate, ">");
				break;
			case '2':
				if (*(c + 1) == '>')
				{
					parsed_redirects[argc] = "2>";
					separator_ret = strsep(&string2separate, "2>");
				}
				break;
			default:
				separator_ret = NULL;
				break;
				// "in <out 2> "
				// " 1   2   2
			}
			if (separator_ret != NULL)
			{
				if (strlen(separator_ret) > 0)
				{
					parsed_redirects[argc] = separator_ret;
					if (new_arg_flag)
						argc++;
					else
						new_arg_flag = 1;
				}
				redirect_sign = (char *)malloc(2);
				redirect_sign[0] = *c;
				redirect_sign[1] = '\0';
				if (new_arg_flag)
					argc++;
				else
					new_arg_flag = 1;
				c = string2separate;
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
	parsed_redirects[argc] = NULL;
	handle_redirect(parsed_redirects);
}				

void handle_redirect(char **parsed_redirects)
{
	char **arg = parsed_redirects;
	redirection_file_stream.error_stream = NULL;
	redirection_file_stream.input_stream = NULL;
	redirection_file_stream.output_stream = NULL;
	int argc = 0, arg_end = 0;
	for (; *arg != NULL; arg++)
	{
		printw("handle_redirect arg: %s\n", *arg);
		switch (**arg)
		{
		case '<':
			redirection_file_stream.input_stream = *(arg + 1);
			arg_end = argc;
			break;
		case '>':
			redirection_file_stream.output_stream = *(arg + 1);
			arg_end = argc;
			break;
		case '2':
			redirection_file_stream.error_stream = *(arg + 1);
			arg_end = argc;
			break;
		default:
			break;
		}
		argc++;
	}
	if (arg_end != 0)
		parsed_redirects[argc] = NULL;
	return;
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
	// TODO: command not found error being added to the history
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
