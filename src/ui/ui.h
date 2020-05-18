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

#ifndef UI_H
#define UI_H

#include <pthread.h>
#include "external/iniparser/dictionary.h"

typedef void * (*ui_f_setup)(const char *);
typedef void (*ui_f_loop)(void*);
typedef void (*ui_f_stop)(void*);
typedef void (*ui_f_destroy)(void*);

struct ui_drv {
	const char *name;
	ui_f_setup setup;
	ui_f_loop loop;
	ui_f_stop stop;
	ui_f_destroy destroy;
};

struct ui {
	struct ui_drv *drv;
	pthread_t th;
	void *data;
};

struct ui * ui_create(dictionary *cfg);
int ui_run(struct ui *ui);
void ui_shutdown(struct ui *ui);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
