#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "em400_debuger.h"

extern int em400_quit;

// -----------------------------------------------------------------------
int em400_debuger_c_quit(char* args)
{
	em400_quit = 1;
	return 1;
}

// -----------------------------------------------------------------------
int em400_debuger_c_step(char* args)
{
	return 1;
}

// -----------------------------------------------------------------------
int em400_debuger_c_help(char* args)
{
	printf("This is help.\n");
	return 0;
}

// -----------------------------------------------------------------------
int em400_debuger_c_regs(char* args)
{
	printf("regs:\n");
	return 0;
}

// -----------------------------------------------------------------------
cmd_s em400_debuger_commands[] = {
	{ "quit",	em400_debuger_c_quit,	"Quit the emulator" },
	{ "step",	em400_debuger_c_step,	"Execute next instruction" },
	{ "help",	em400_debuger_c_help,	"Print help" },
	{ "?",		em400_debuger_c_help,	"Synonym for 'help'" },
	{ "regs",	em400_debuger_c_regs,	"Print registers" },
	{ NULL,		NULL,	NULL }
};

// -----------------------------------------------------------------------
int em400_debuger_execute(char* line)
{
	char cmd[10+1] = {0};
	char args[100+1] = {0};
	int res;

	res = sscanf(line, "%10s %100s", cmd, args);

	cmd_s* cmd_pos = em400_debuger_commands;

	while (cmd_pos->cmd) {
		if (!strcmp(cmd, cmd_pos->cmd)) {
			return cmd_pos->fun(args);
		}
		cmd_pos++;
	}
	printf("Unknown command: %s\n", cmd);
	return 0;
}
 
// -----------------------------------------------------------------------
void em400_debuger_step()
{
	char *buf;
	int done = 0;
 
	rl_bind_key('\t', rl_abort); //disable auto-complete

	while (!done)  {
		buf = readline("em400> ");
		if (!buf) {
			printf("\n");
			break;
		} else {
			if (*buf != 0) {
				add_history(buf);
				if (em400_debuger_execute(buf)) {
					done = 1;
				}
			}
		}
		free(buf);
	}
}

