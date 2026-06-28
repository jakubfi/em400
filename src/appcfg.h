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

// id given to the single machine imported from the old (one-machine) INI format
#define APPCFG_IMPORTED_MACHINE_ID "imported"

struct appcfg_machine {
	char *id;
	char *name;
	struct em400_machine_cfg cfg;
};

// app-owned logging mirror: persisted for round-trip Save, never handed to the
// library (logging keeps its own lifecycle/live API, see config-design.md)
struct appcfg_log {
	bool enabled;
	char *components;
	char *file;
	bool line_buffered;
};

struct appcfg {
	struct appcfg_machine *machines;
	int n_machines;
	int cap_machines;
	char *active_id;
	struct appcfg_log log;
	struct em400_host_cfg host;
};

extern struct appcfg appcfg;

#ifdef __cplusplus
extern "C" {
#endif

int appcfg_build_from_ini(em400_cfg *cfg);
int appcfg_write(const struct appcfg *c, const char *path);
int appcfg_copy(struct appcfg *dst, const struct appcfg *src);
void appcfg_set_path(const char *path);
const char * appcfg_path(void);
void appcfg_free_contents(struct appcfg *c);
void appcfg_free(void);

struct appcfg_machine *appcfg_machine_add(struct appcfg *c, const char *id, const char *name);
void appcfg_machine_delete(struct appcfg *c, const char *id);
struct appcfg_machine *appcfg_machine_find(struct appcfg *c, const char *id);
struct em400_machine_cfg *appcfg_active_machine(struct appcfg *c);
void appcfg_machine_set_name(struct appcfg_machine *m, const char *name);
bool appcfg_set_image(struct appcfg_machine *m, unsigned chan, unsigned dev, unsigned slot, const char *path);

#ifdef __cplusplus
}
#endif

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
