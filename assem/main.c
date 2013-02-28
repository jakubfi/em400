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
#include <getopt.h>

#include "elements.h"
#include "eval.h"

extern FILE *yyin;
extern int got_error;
int yyparse();

// -----------------------------------------------------------------------
int parse(FILE *source)
{
	yyin = source;

	do {
		 yyparse();
	} while (!feof(yyin));

	if (got_error) {
		printf("Error parsing source. Exiting.\n");
		return -1;
	}

	return 1;
}

// -----------------------------------------------------------------------
int assembly(struct word_t *word, uint16_t *outdata)
{
	int wcounter = 0;
	int res;

	while (word) {
		res = make_bin(wcounter, word, outdata);
		if (res < 0) {
			printf("Error assembling binary image, line %i: %s\nExiting.\n", word->lineno-1, assembly_error);
			return -1;
		}
		wcounter += res;
		word = word->next;
	}

	return wcounter;
}

// -----------------------------------------------------------------------
void usage()
{
	printf("Usage: assem [-k] [-c] <file.asm>\n");
}

// -----------------------------------------------------------------------
int main(int argc, char **argv)
{
	int option;
	int k202 = 0;
	int classic = 0;

	while ((option = getopt(argc, argv,"kc")) != -1) {
		switch (option) {
			case 'k':
				k202 = 1;
				break;
			case 'c':
				classic = 1;
				break;
			default:
				usage(); 
				exit(1);
		}
	}

	if (optind != argc-1) {
		usage();
		exit(1);
	}

	// read input file
	FILE *asm_source = fopen(argv[optind], "r");
	if (!asm_source) {
		printf("Cannot open input file\n");
		exit(1);
	}

	// parse program
	int res = parse(asm_source);

	fclose(asm_source);

	if (res < 0) {
		exit(1);
	}

	// assembly binary image
	uint16_t outdata[MAX_PROG_SIZE+4];
	int wcounter = assembly(program_start, outdata);

	if (wcounter < 0) {
		exit(1);
	}

	// write output program
	FILE *bin_out = fopen("test.bin", "w");
	res = fwrite(outdata, 2, wcounter, bin_out);
	fclose(bin_out);
	printf("Program size: %i, words written: %i\n", wcounter, res);
	if (wcounter != res) {
		printf("Words assembled and written differ, binary file is broken.\n");
		exit(1);
	}

	return 0;
}

// vim: tabstop=4
