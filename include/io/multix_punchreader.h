//  Copyright (c) 2013 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef _MULTIX_PUNCHREADER_H_
#define _MULTIX_PUNCHREADER_H_

#include <inttypes.h>
#include <pthread.h>

#include "io/multix.h"

struct mx_unit_punchreader_t {
	struct mx_unit_proto_t proto;
};

struct mx_unit_proto_t * mx_punchreader_create(struct cfg_arg *args);
struct mx_unit_proto_t * mx_punchreader_create_nodev();
void mx_punchreader_connect(struct mx_unit_punchreader_t *unit);
void mx_punchreader_disconnect(struct mx_unit_punchreader_t *unit);
void mx_punchreader_shutdown(struct mx_unit_proto_t *unit);
void mx_punchreader_reset(struct mx_unit_proto_t *unit);
int mx_punchreader_cfg_phy(struct mx_unit_proto_t *unit, struct mx_cf_sc_pl *cfg_phy);
int mx_punchreader_cfg_log(struct mx_unit_proto_t *unit, struct mx_cf_sc_ll *cfg_log);
uint16_t mx_punchreader_get_status(struct mx_unit_proto_t *unit);

void mx_punchreader_cmd_attach(struct mx_unit_proto_t *unit, uint16_t addr);
void mx_punchreader_cmd_detach(struct mx_unit_proto_t *unit, uint16_t addr);
void mx_punchreader_cmd_transmit(struct mx_unit_proto_t *unit, uint16_t addr);

struct mx_punchreader_cf_t * mx_punchreader_cf_t_decode(int addr);
void mx_punchreader_cf_t_free(struct mx_punchreader_cf_t *cf);



#endif

// vim: tabstop=4 shiftwidth=4 autoindent
