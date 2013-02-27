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

extern struct word_t *program_start;
extern struct word_t *program_end;

extern char assembly_error[];
extern int ic;

void program_append(struct word_t *word);
int make_bin(int ic, struct word_t *word, uint16_t *dt);
struct enode_t * enode_eval(struct enode_t *e);

#endif

// vim: tabstop=4
