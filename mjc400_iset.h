#include <inttypes.h>
#include <stdbool.h>

typedef struct {
	uint16_t opcode;
	char mnemonic[3];
	_Bool nl;
	int (*op_fun)();

} mjc400_opdef;

extern mjc400_opdef mjc400_iset[];

