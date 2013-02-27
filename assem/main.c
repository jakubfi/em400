//  Copyright (c) 2013 Jakub Filipowicz <jakubf@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include <stdio.h>
#include <stdlib.h>

#include "elements.h"
#include "eval.h"

extern FILE *yyin;
extern int got_error;
int yyparse();

// -----------------------------------------------------------------------
int main(void) {

	FILE *asm_source = fopen("test.asm", "r");
	if (!asm_source) {
		printf("Cannot open input file\n");
		exit(1);
	}
	yyin = asm_source;
	do {
		 yyparse();
	} while (!feof(yyin));
	fclose(asm_source);

	if (got_error) {
		printf("Exiting.\n");
		exit(1);
	}

	uint16_t outdata[MAX_PROG_SIZE+4];
	struct word_t *word = program_start;
	int wcounter = 0;
	int res;
	while (word) {
		res = make_bin(wcounter, word, outdata);
		if (res < 0) {
			printf("Error assembling binary image, line %i: %s\nExiting.\n", word->lineno-1, assembly_error);
			exit(1);
		}
		wcounter += res;
		word = word->next;
	}

	FILE *bin_out = fopen("test.bin", "w");
	res = fwrite(outdata, 2, wcounter, bin_out);
	fclose(bin_out);
	printf("Program size: %i, words written: %i\n", wcounter, res);

	return 0;
}

// vim: tabstop=4
