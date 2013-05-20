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

#ifndef ELEMENTS_H
#define ELEMENTS_H

#include "nodes.h"

extern struct nodelist_t *program;

struct node_t * mknod_valstr(int type, int value, char *str);
struct node_t * mknod_nargs(int type, struct node_t *n1, struct node_t *n2);
struct node_t * mknod_op(int optype, int op, int ra, struct node_t *arg, struct norm_t *norm);
struct node_t * mknod_dentry(int type, char *name, struct node_t *value);
struct node_t * mknod_file(char *name, char *type, struct node_t *addr, struct node_t *attr);

#endif

// vim: tabstop=4
