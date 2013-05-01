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

#include "parsers.h"
#include "keywords.h"
#include "eval.h"
#include "pprocess.h"
#include "image.h"
#include "errors.h"

int preprocessor = 0;
char *input_file = NULL;
char *output_file = NULL;

// -----------------------------------------------------------------------
void usage()
{
	printf("Usage: assem [-d] [-k] [-c] [-p [-2]] <input.asm> [output]\n");
	printf("Where:\n");
	printf("   -d : Enable debug messages (lots of)\n");
	printf("   -k : use K-202 mnemonics (instead of MERA-400)\n");
	printf("   -c : use classic ASSK/ASSM syntax (instead of modern)\n");
	printf("   -p : produce preprocessor output (.pp.asm file)\n");
	printf("   -2 : use K-202 mnemonics in preprocessor output (instead of MERA-400)\n");
}

// -----------------------------------------------------------------------
void parse_args(int argc, char **argv)
{
	mnemo_sel = MERA400;
	pp_mnemo_sel = MERA400;
	syntax = MODERN;
	preprocessor = 0;
	enable_debug = 0;

	int option;

	// parse options
	while ((option = getopt(argc, argv,"dkchp2:")) != -1) {
		switch (option) {
			case 'd':
				enable_debug = 1;
				break;
			case 'h':
				usage();
				exit(0);
			case 'k':
				mnemo_sel = K202;
				break;
			case 'c':
				syntax = CLASSIC;
				break;
			case 'p':
				preprocessor = 1;
				break;
			case '2':
				pp_mnemo_sel = K202;
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
		output_file = strdup(argv[optind+1]);
	} else {
		usage();
		exit(1);
	}

	if (!strcmp(input_file, output_file)) {
		printf("Error: input and output files are the same, exiting.\n");
		exit(1);
	}
}

// -----------------------------------------------------------------------
int main(int argc, char **argv)
{
	parse_args(argc, argv);

	// open input file
	FILE *asm_source = fopen(argv[optind], "r");
	if (!asm_source) {
		printf("Error: cannot open input file, exiting.\n");
		exit(1);
	}

	// parse program
	int res = parse(asm_source);

	fclose(asm_source);

	if (res < 0) {
		printf("Cannot parse source, exiting.\n");
		nodelist_drop(program);
		exit(1);
	}

	// assembly binary image
	res = assembly(program);

	if (res < 0) {
		printf("Error at %s\n", assembly_error);
		nodelist_drop(program);
		exit(1);
	}

	res = img_write(output_file);

	if (res < 0) {
		if (res == E_IO_OPEN) {
			printf("Cannot open output file '%s', exiting.\n", output_file);
		} else if (res == E_IO_WRITE) {
			printf("Error: not all words written, output file '%s' is broken.\n", output_file);
		} else {
			printf("Unknown error during image write\n");
		}
	} else {
		printf("Written %i words to file '%s'.\n", res, output_file);
	}

	// write preprocessor output
	if (preprocessor) {
		char *pp_file = malloc(strlen(output_file)+10);
		sprintf(pp_file, "%s.pp.asm", output_file);
		printf("Writing preprocessor output to: %s\n", pp_file);
		FILE *ppf = fopen(pp_file, "w");
		if (!ppf) {
			printf("Cannot open preprocessor output file '%s', sorry.\n", pp_file);
		}
		preprocess_new(program, ppf);
		fclose(ppf);
		free(pp_file);
	}

	free(output_file);
	nodelist_drop(program);

	return 0;
}

// vim: tabstop=4
