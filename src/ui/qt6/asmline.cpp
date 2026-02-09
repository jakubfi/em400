#include "asmline.h"

// -----------------------------------------------------------------------
AsmLine::AsmLine(int nb, int addr, int length, const char *text) :
	nb(nb),
	addr(addr),
	length(length),
	text(text)
{

}

// -----------------------------------------------------------------------
AsmLine::AsmLine(int nb, int addr, int length, QString text) :
	nb(nb),
	addr(addr),
	length(length),
	text(text)
{

}

// -----------------------------------------------------------------------
AsmLine::~AsmLine()
{

}
