#include "ysh.h"

int main()
{
	char input_string[MAX_COMMAND_LENGTH] = {0}, *parsed_args[MAX_COMMANDS];
	char *parsed_args_piped[MAX_COMMANDS];
	command_type exec_flag = 0;
	int exit = 0;
	int ch;
	init_shell(); /* Start curses mode 		  */
	HIST_ENTRY *history_entry;

	while (1)
	{
		print_usr_dir();

		if (read_input(input_string))
			continue;

		exec_flag = process_input_string(input_string,
																		 parsed_args, parsed_args_piped);

		switch (exec_flag)
		{
		case SIMPLE:
			execArgs(parsed_args);
			break;
		case PIPE:
			execArgsPiped(parsed_args, parsed_args_piped);
			break;
		default:
			break;
		}

		ch = input_string[0];

		// buffer verification for arrow keys and signals
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
		if (exit)
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
	// TODO: open redirection file
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

int read_input(char *input_string)
{
	char buf[MAX_COMMAND_LENGTH];

	getstr(buf);

	mvprintw(20, 20, "buf: %s", buf);
	if (strlen(buf) != 0)
	{
		add_history(buf);
		strcpy(input_string, buf);
		return 0;
	}
	else
		return 1;
}

command_type process_input_string(char *input_string, char **parsed_args, char **parsed_args_piped)
{
	char *input_string_piped[2];
	int piped = 0;

	piped = parse_pipe(input_string, input_string_piped);

	if (piped)
	{
		parse_whitespaces(input_string_piped[0], parsed_args);
		parse_whitespaces(input_string_piped[1], parsed_args_piped);
	}
	else
		parse_whitespaces(input_string, parsed_args);

	if (handle_builtin_command(parsed_args))
		return NO_COMMAND_OR_BUILTIN;
	else
		return SIMPLE + piped;
}

int parse_pipe(char *input_string, char **input_string_piped)
{
	for (int i = 0; i < 2; i++)
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

// TODO: complete commands
command_type handle_builtin_commands(char **parsed_args)
{
	builtin_command current_command = match_command(parsed_args[0]);

	switch (current_command)
	{
	case BG:
		break;
	case CD:
		change_dir(parsed_args[1]);
		break;
	case ECHO:
		printw("\n%s\n", parsed_args[1]);
		break;
	case EXIT:
		exit(EXIT_SUCCESS);
		break;
	case EXPORT:
		break;
	case FG:
		break;
	case HELP:
		print_help();
		break;
	case HISTORY:
		break;
	case JOBS:
		break;
	case KILL:
		break;
	case SET:
		break;
	default:
		return 0;
	}

	return 1;
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

void destroy_shell()
{
	// dumps the current history to the file ~/.history
	write_history(NULL);
	endwin(); /* End curses mode		  */
}
