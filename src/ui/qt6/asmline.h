#include <QString>

#ifndef ASMLINE_H
#define ASMLINE_H

// -----------------------------------------------------------------------
class AsmLine {

public:
	int nb;
	int addr;
	int length;
	QString text;
	AsmLine(int nb, int addr, int length, const char *text);
	AsmLine(int nb, int addr, int length, QString text);
	~AsmLine();
};

#endif // ASMLINE_H
