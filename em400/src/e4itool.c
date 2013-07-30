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

#include "e4image.h"

int main(int argc, char **argv)
{
	int res;
	struct e4i_t *i;

	i = e4i_create_chs("test.e4i", 10, 20, 1, 1, 1);
	printf("create: (%i) %s\n", e4i_err, e4i_get_err(e4i_err));

	if (i) {
		e4i_header_print(i);
		res = e4i_import(i, "source", E4I_T_HDD, 0);
		printf("import: (%i) %s\n", res, e4i_get_err(res));
		e4i_header_print(i);
		e4i_close(i);
	}

	i = e4i_open("test.e4i");
	printf("open: (%i) %s\n", e4i_err, e4i_get_err(e4i_err));

	if (i) {
		e4i_header_print(i);
	}

	return 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
