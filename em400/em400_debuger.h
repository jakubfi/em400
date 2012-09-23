#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

typedef struct {
	char *cmd;
	int (*fun)(char*);
	char *doc;
} cmd_s;

extern cmd_s em400_debuger_commands[];

int em400_debuger_c_quit(char* args);
int em400_debuger_c_step(char* args);
int em400_debuger_c_help(char* args);
int em400_debuger_c_regs(char* args);

int em400_debuger_execute(char* line);
void em400_debuger_step();
