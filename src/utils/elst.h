//  Copyright (c) 2018 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef __ELST_H__
#define __ELST_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct elst *ELST;

ELST elst_create(int capacity);
void elst_destroy(ELST l);

void elst_nlock_clear(ELST l);
int elst_nlock_count(ELST l);
int elst_nlock_append(ELST l, void *ptr);
int elst_nlock_prepend(ELST l, void *ptr);
int elst_nlock_insert(ELST l, void *ptr, int prio);
void * elst_nlock_pop(ELST l);

void elst_clear(ELST l);
int elst_count(ELST l);
int elst_append(ELST l, void *ptr);
int elst_prepend(ELST l, void *ptr);
int elst_insert(ELST l, void *ptr, int prio);
void * elst_pop(ELST l);
void * elst_wait_pop(ELST l, int timeout);

#ifdef __cplusplus
}
#endif

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
