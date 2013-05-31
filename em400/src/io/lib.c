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

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "io/drivers.h"

// -----------------------------------------------------------------------
void chan_get_int_spec(void *self, uint16_t *r)
{
	struct chan_t *ch = self;
	*r = ch->int_spec;
}

// -----------------------------------------------------------------------
int args_to_cfg(struct cfg_arg_t *arg, const char *format, ...)
{
	int *i;
	char *c;
	char **s;
	char **eptr = NULL;
	int count = 0;

	if (!format) return -1;

	va_list ap;
	va_start(ap, format);
	while (arg && *format) {
		if (!arg->text) return -1;
		switch (*format) {
			case 'i':
				i = va_arg(ap, int*);
				*i = strtol(arg->text, eptr, 10);
				if (eptr) return -1;
				break;
			case 'c':
				c = va_arg(ap, char*);
				*c = *(arg->text);
				break;
			case 's':
				s = va_arg(ap, char**);
				*s = strdup(arg->text);
				if (!*s) return -1;
				break;
			default:
				return -1;
		}
		count++;
		format++;
		free(arg->text);
		struct cfg_arg_t *parg = arg;
		arg = arg->next;
		free(parg);
	}

	va_end(ap);
	return count;
}

// vim: tabstop=4
