//  Copyright (c) 2012-2013 Jakub Filipowicz <jakubf@gmail.com>
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

#include "cfg.h"
#include "cfg_parser.h"

extern FILE *cyyin;
int cyyparse();
int cfg_error = 0;

// -----------------------------------------------------------------------
int cfg_load(char *cfg_file)
{
	FILE * cfg = fopen(cfg_file, "r");
	if (!cfg) {
		return -1;
	}

	cyyin = cfg;
	do {
		cyyparse();
	} while (!feof(cyyin));

	fclose(cfg);
	if (cfg_error) return -1;
	return 0;
}

// -----------------------------------------------------------------------
int cfg_set_mem(int module, int is_mega, int segments)
{
	if ((module < 0) || (module > 15)) {
		return -1;
	}
	if ((segments < 1) || (segments > 16)) {
		return -2;
	}
	if ((module == 0) && (segments < 2)) {
		return -3;
	}
	em400_cfg.mem[module].segments = segments;
	em400_cfg.mem[module].is_mega = is_mega;
	return 0;
}


// vim: tabstop=4
