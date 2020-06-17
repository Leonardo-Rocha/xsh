#include "ysh.h"

int main()
{
	char input_string[MAX_COMMAND_LENGTH]; // *parsed_args[MAX_COMMANDS_HISTORY];
	// char* parsedArgsPiped[MAX_COMMANDS_HISTORY];
	// int execFlag = 0;
	int exit = 0;
	int ch;
	init_shell(); /* Start curses mode 		  */
	HIST_ENTRY *history_entry;

	while (1)
	{
		print_usr_dir();

		ch = getch();

		// buffer verification for arrow keys and quit signal
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

		if (read_input(input_string))
			continue;
			
		refresh();
	}

	destroy_shell();

	return 0;
}

void init_shell()
{
	initscr();						 // Start curses mode
	raw();								 // Line buffering disabled
	halfdelay(20);				 // When we do getch(), wait for 2 seconds before return ERR value
	keypad(stdscr, TRUE);  // enables keypad to use the arrow keys to scroll on the process list
	scrollok(stdscr, TRUE);// enables scroll
	idlok(stdscr, TRUE);
	// read if there's a history in ~/.history
	read_history(NULL);
}

void print_usr_dir()
{
	char *username = getenv("USER");
	char hostname[32];
	int hostname_ret = gethostname(hostname, sizeof(hostname));
	char cwd[1024];
	char *cwd_ret = getcwd(cwd, sizeof(cwd));
	char *home = getenv("HOME");
	int home_length = strlen(home);

	// TODO: proper error handling
	if (cwd_ret == NULL || hostname_ret != 0)
		exit(1);

	printw("%s@%s:", username, hostname);

	// TODO: tint with some colors to make it more beautiful
	// change the /home/username to ~/
	if (strncasecmp(cwd, home, home_length) == 0)
	{
		char *c = (cwd + home_length);
		printw("~%s $ ", c);
	}

	refresh();
}

int read_input(char *str)
{
	char buf[MAX_COMMAND_LENGTH];

	getstr(buf);

	printf("buf: %s", buf);
	if (strlen(buf) != 0)
	{
		add_history(buf);
		strcpy(str, buf);
		return 0;
	}
	else
		return 1;
}

// TODO: Add parse run
void run_foreground(const char* input_sequence[], char* exec_input, char* exec_output, char* exec_error)
{
	char* file = input_sequence[0];
	pid_t pid = fork();
	if(pid = 0)
		return;
	else if(pid = -1)
		// TODO: FORK ERROR HANDLING
		return;
	if(exec_input != NULL)
		freopen(exec_input, "r", stdin);
	if(exec_output != NULL)
		freopen(exec_output, "w", stdout);
	if(exec_error != NULL)
		freopen(exec_error, "w", stderr);
	execvp(file, input_sequence);
}

void run_background(const char* input_sequence[], char* exec_input, char* exec_output, char* exec_error)
{
	char* file = input_sequence[0];
	pid_t pid = fork();
	if(pid = 0)
		return;
	else if(pid = -1)
		// TODO: FORK ERROR HANDLING
		return;
	// stdout and stderr redirection
	if(exec_input != NULL)
		freopen(exec_input, "r", stdin);
	else
		freopen(BACKGROUND_IN, "r", stdin);
	if(exec_output != NULL)	
		freopen(exec_output, "w", stdout);
	else
		freopen(BACKGROUND_OUT, "r", stdout);
	if(exec_error != NULL)	
		reopen(exec_error, "w", stderr);
	else
		freopen(BACKGROUND_ERROR, "r", stderr);
	execvp(file, input_sequence);
}


void destroy_shell()
{
	// dumps the current history to the file ~/.history
	write_history(NULL);
	endwin(); /* End curses mode		  */
}
