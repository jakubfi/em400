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

#ifndef APPCFG_H
#define APPCFG_H

#include "libem400.h"
#include "cfg.h"

// for the old (one-machine) INI format
#define APPCFG_DEFAULT_MACHINE_ID "default"

struct appcfg_machine {
	char *id;
	char *name;
	struct em400_machine_cfg cfg;
};

struct appcfg {
	struct appcfg_machine *machines;
	int n_machines;
	int cap_machines;
	char *active_id;
	struct em400_host_cfg host;
};

extern struct appcfg appcfg;

int appcfg_build_from_ini(em400_cfg *cfg);
int appcfg_write(const struct appcfg *c, const char *path);
void appcfg_free(void);

struct appcfg_machine *appcfg_machine_add(struct appcfg *c, const char *id, const char *name);
void appcfg_machine_delete(struct appcfg *c, const char *id);
struct appcfg_machine *appcfg_machine_find(struct appcfg *c, const char *id);
struct em400_machine_cfg *appcfg_active_machine(struct appcfg *c);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
