//  Copyright (c) 2026 Jakub Filipowicz <jakubf@gmail.com>
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
#include <stdarg.h>

#include "libem400.h"
#include "app_err.h"

// -----------------------------------------------------------------------
int app_errf(const char *func, const char *fmt, ...)
{
	char buf[512];

	va_list vl;
	va_start(vl, fmt);
	vsnprintf(buf, sizeof buf, fmt, vl);
	va_end(vl);

	fprintf(stderr, "%s\n", buf);
	em400_logf(func, "ERROR: %s", buf);

	return E_ERR;
}

// -----------------------------------------------------------------------
void app_msg_drain(void)
{
	const char *text = em400_msg_take(NULL);
	if (text) {
		fprintf(stderr, "%s\n", text);
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
