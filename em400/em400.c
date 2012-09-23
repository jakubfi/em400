//  Copyright (c) 2011-2012 Jakub Filipowicz <jakubf@gmail.com>
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
#include "mjc400.h"
#include "em400_debuger.h"

int em400_quit = 0;

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(int argc, char** argv)
{
	mjc400_init();

	mjc400_load_os_image("add", 0);
	while (!em400_quit) {
		em400_debuger_step();
		if (!mjc400_step()) {
			em400_quit = 1;
		}
	}
	mjc400_dump_os_mem();

	return 0;
}
