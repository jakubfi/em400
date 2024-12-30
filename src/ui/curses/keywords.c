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
#include <strings.h>

#include "ui_curses_parser.h"
#include "ui/curses/keywords.h"
#include "libem400.h"

struct kw_t kw_regs[] = {
	{ "IC", EM400_REG_IC },
	{ "SR", EM400_REG_SR },
	{ "IR", EM400_REG_IR },
	{ "AR", EM400_REG_AR },
	{ "AC", EM400_REG_AC },
	{ "KB",	EM400_REG_KB },
	{ NULL, 0 }
};

struct kw_t kw_bases[] = {
	{ "HEX", HEX },
	{ "OCT", OCT },
	{ "BIN", BIN },
	{ "INT", INT },
	{ "UINT", UINT },
	{ NULL, 0 }
};


// -----------------------------------------------------------------------
int find_token(struct kw_t *tok_table, const char *text)
{
	struct kw_t *t = tok_table;
	while (t && t->name) {
		if (!strcasecmp(text, t->name)) {
			return t->value;
		}
		t++;
	}
	return 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
