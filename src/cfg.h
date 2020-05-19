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

#ifndef _CFG_H_
#define _CFG_H_

#include "external/iniparser/iniparser.h"
#include "cfg.h"

typedef dictionary em400_cfg;

#define cfg_getdouble iniparser_getdouble
#define cfg_getint iniparser_getint
#define cfg_getstr iniparser_getstring
#define cfg_getbool iniparser_getboolean
#define cfg_contains iniparser_find_entry
#define cfg_set iniparser_set
#define cfg_load iniparser_load
#define cfg_free iniparser_freedict

#endif
