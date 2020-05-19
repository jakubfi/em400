//  Copyright (c) 2020 Jakub Filipowicz <jakubf@gmail.com>
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

#define MAX_KEY_LEN 128

#include <stdarg.h>

#include "cfg.h"

// -----------------------------------------------------------------------
const char * cfg_fgetstr(em400_cfg *cfg, const char *key_format, ...)
{
	char key[MAX_KEY_LEN];
	va_list vl;
	va_start(vl, key_format);
	vsnprintf(key, MAX_KEY_LEN, key_format, vl);
	va_end(vl);
	return cfg_getstr(cfg, key, NULL);
}

// -----------------------------------------------------------------------
int cfg_fgetint(em400_cfg *cfg, const char *key_format, ...)
{
	char key[MAX_KEY_LEN];
	va_list vl;
	va_start(vl, key_format);
	vsnprintf(key, MAX_KEY_LEN, key_format, vl);
	va_end(vl);
	return cfg_getint(cfg, key, -1);
}

// -----------------------------------------------------------------------
double cfg_fgetdouble(em400_cfg *cfg, const char *key_format, ...)
{
	char key[MAX_KEY_LEN];
	va_list vl;
	va_start(vl, key_format);
	vsnprintf(key, MAX_KEY_LEN, key_format, vl);
	va_end(vl);
	return cfg_getdouble(cfg, key, -1);
}

// -----------------------------------------------------------------------
int cfg_fgetbool(em400_cfg *cfg, const char *key_format, ...)
{
	char key[MAX_KEY_LEN];
	va_list vl;
	va_start(vl, key_format);
	vsnprintf(key, MAX_KEY_LEN, key_format, vl);
	va_end(vl);
	return cfg_getbool(cfg, key, -1);
}

// -----------------------------------------------------------------------
int cfg_fcontains(em400_cfg *cfg, const char *key_format, ...)
{
	char key[MAX_KEY_LEN];
	va_list vl;
	va_start(vl, key_format);
	vsnprintf(key, MAX_KEY_LEN, key_format, vl);
	va_end(vl);
	return cfg_contains(cfg, key);
}
