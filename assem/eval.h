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

#ifndef EVAL_H
#define EVAL_H

#include <inttypes.h>

#define MAX_PROG_SIZE 64*1024

extern struct node_t *program_start;
extern struct node_t *program_end;
extern struct dict_t **dict;
extern char assembly_error[];
extern int ic;

int program_append(struct node_t *word);
int compose_data(int ic, struct node_t *word, uint16_t *dt);
int compose_opcode(int ic, struct node_t *word, uint16_t *dt);
struct node_t * enode_eval(struct node_t *n, char *refcheck);

#endif

// vim: tabstop=4
