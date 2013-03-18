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
#include <string.h>
#include <getopt.h>

#include "ops.h"
#include "parser_modern.h"
#include "elements.h"
#include "eval.h"

int classic = 0;

extern FILE *m_yyin;
extern FILE *c_yyin;
extern int got_error;
int m_yyparse();
int c_yyparse();

// -----------------------------------------------------------------------
int parse(FILE *source)
{
	m_yyin = c_yyin = source;

	int (*active_parser)();
	if (classic) active_parser = c_yyparse;
	else active_parser = m_yyparse;

	do {
		active_parser();
	} while (!feof(source));

	if (got_error) {
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
		if (word->type == O_DATA) {
			res = compose_data(wcounter, word, outdata);
		} else {
			res = compose_opcode(wcounter, word, outdata);
		}
		if (res < 0) {
			return -wcounter;
		}
		wcounter += res;
		word = word->next;
	}

	return wcounter;
}

// -----------------------------------------------------------------------
void usage()
{
	printf("Usage: assem [-k] [-c] <input.asm> [output]\n");
	printf("Where:\n");
	printf("   -k : use K-202 mnemonics (instead of MERA-400)\n");
	printf("   -c : use classic ASSK/ASSM syntax (instead of modern)\n");
}

// -----------------------------------------------------------------------
int main(int argc, char **argv)
{
	int option;
	mnemo_sel = MNEMO_MERA400;
	classic = 0;
	char *input_file = NULL;
	char *output_file = NULL;

	// parse options
	while ((option = getopt(argc, argv,"kch")) != -1) {
		switch (option) {
			case 'h':
				usage();
				exit(0);
			case 'k':
				mnemo_sel = MNEMO_K202;
				break;
			case 'c':
				classic = 1;
				break;
			default:
				usage();
				exit(1);
		}
	}

	// parse arguments
	if (optind == argc-1) {
		input_file = argv[optind];
		output_file = strdup(input_file);
		char *pos = strstr(output_file, ".asm\0");
		if (!pos) {
			printf("Error: input file is not .asm, exiting.\n");
			exit(1);
		}
		strcpy(pos, ".bin");
	} else if (optind == argc-2) {
		input_file = argv[optind];
		output_file = argv[optind+1];
	} else {
		usage();
		exit(1);
	}

	if (!strcmp(input_file, output_file)) {
		printf("Error: input and output files are the same, exiting.\n");
		exit(1);
	}

	// read input file
	FILE *asm_source = fopen(argv[optind], "r");
	if (!asm_source) {
		printf("Error: cannot open input file, exiting.\n");
		exit(1);
	}

	// open output file
	FILE *bin_out = fopen(output_file, "w");
	if (!bin_out) {
		printf("Cannot open output file '%s', exiting.\n", output_file);
		fclose(asm_source);
		exit(1);
	}

	// parse program
	dict = dict_create();
	int res = parse(asm_source);

	fclose(asm_source);

	if (res < 0) {
		printf("Cannot parse source, exiting.\n");
		program_drop(program_start);
		dict_drop(dict);
		exit(1);
	}

	// assembly binary image
	uint16_t outdata[MAX_PROG_SIZE+4];
	int wcounter = assembly(program_start, outdata);

	if (wcounter == 0) {
		printf("Nothing to assemble, empty program, exiting.\n");
	} else if (wcounter < 0) {
		printf("Error assembling binary image at IC=%i: %s, exiting.\n", -wcounter-1, assembly_error);
	}

	if (wcounter <= 0) {
		program_drop(program_start);
		dict_drop(dict);
		exit(1);
	}

	printf("Assembled %i words\n", wcounter);

	// write output program
	res = fwrite(outdata, 2, wcounter, bin_out);
	fclose(bin_out);
	printf("Written %i words to file '%s'.\n", res, output_file);
	if (wcounter != res) {
		printf("Error: not all words written, output file '%s' is broken.\n", output_file);
		program_drop(program_start);
		dict_drop(dict);
		exit(1);
	}

	program_drop(program_start);
	dict_drop(dict);
	return 0;
}

// vim: tabstop=4
