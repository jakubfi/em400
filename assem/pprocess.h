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

#ifndef PPROCESS_H
#define PPROCESS_H

#include <inttypes.h>

char * pp_get_labels(struct dict_t **dict, int addr);
char * pp_get_mnemo(struct node_t *n);
int pp_compose_opcode(int ic, struct node_t *word, FILE *ppf);
char * pp_expr_eval(struct node_t *n);

#endif

// vim: tabstop=4
