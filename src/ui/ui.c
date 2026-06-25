//  Copyright (c) 2016 Jakub Filipowicz <jakubf@gmail.com>
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

#define _XOPEN_SOURCE 500
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "libem400.h"
#include "app_err.h"
#include "ui/ui.h"

extern struct ui_drv ui_cmd;
extern struct ui_drv ui_qt6;

// in order of preference
struct ui_drv* uis[] = {
#ifdef UI_QT
	&ui_qt6,
#endif
	&ui_cmd,
	NULL
};

// -----------------------------------------------------------------------
void ui_print_uis(FILE *fd)
{
	struct ui_drv **u = uis;
	while (*u) {
		if (u != uis) fprintf(fd, ",");
		fprintf(fd, " %s", (*u)->name);
		u++;
	}
}

// -----------------------------------------------------------------------
struct ui * ui_create(const char *name)
{
	if (!name) {
		name = uis[0]->name;
	}

	struct ui_drv **drv = uis;
	while (drv && *drv) {
		if (!strncasecmp(name, (*drv)->name, strlen((*drv)->name))) {
			struct ui *ui = (struct ui *) calloc(1, sizeof(struct ui));
			if (!ui) {
				app_err("Memory allocation error when creating UI.");
				return NULL;
			}
			ui->drv = *drv;

			// setup the UI
			ui->data = ui->drv->setup(name);
			if (!ui->data) {
				app_err("Failed to setup UI: %s.", name);
				return NULL;
			} else {
				em400_log("UI started: %s", name);
			}
			return ui;
		}
		drv++;
	}

	app_err("Unknown UI: %s.", name);
	return NULL;
}

// -----------------------------------------------------------------------
int ui_run(struct ui *ui)
{
	ui->drv->loop(ui->data);

	return E_OK;
}

// -----------------------------------------------------------------------
void ui_shutdown(struct ui *ui)
{
	if (!ui) {
		return;
	}

	ui->drv->destroy(ui->data);
	free(ui);
}

// vim: tabstop=4 shiftwidth=4 autoindent
