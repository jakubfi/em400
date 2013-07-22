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

#include <stdlib.h>

#include "debugger/log.h"
#include "errors.h"
#include "cpu/memory.h"

#include "io/multix_punchreader.h"

#define UNIT ((struct mx_unit_punchreader_t *)(unit))

// -----------------------------------------------------------------------
struct mx_unit_proto_t * mx_punchreader_create(struct cfg_arg_t *args)
{
	eprint("      Tape punchreader\n");

	struct mx_unit_proto_t *unit = mx_punchreader_create_nodev();
	if (!unit) {
		gerr = E_ALLOC;
		return NULL;
	}

	mx_punchreader_connect(UNIT);
	return unit;
}

// -----------------------------------------------------------------------
struct mx_unit_proto_t * mx_punchreader_create_nodev()
{
	struct mx_unit_punchreader_t *unit = calloc(1, sizeof(struct mx_unit_punchreader_t));
	return (struct mx_unit_proto_t *) unit;
}

// -----------------------------------------------------------------------
void mx_punchreader_connect(struct mx_unit_punchreader_t *unit)
{
}

// -----------------------------------------------------------------------
void mx_punchreader_disconnect(struct mx_unit_punchreader_t *unit)
{
}

// -----------------------------------------------------------------------
void mx_punchreader_shutdown(struct mx_unit_proto_t *unit)
{
	mx_punchreader_disconnect(UNIT);
	free(UNIT);
}

// -----------------------------------------------------------------------
void mx_punchreader_reset(struct mx_unit_proto_t *unit)
{

}

// -----------------------------------------------------------------------
int mx_punchreader_cfg_phy(struct mx_unit_proto_t *unit, struct mx_cf_sc_pl *cfg_phy)
{
	LOG(D_IO, 10, "MULTIX/punchreader (line:%i): configure physical line", unit->phy_num);
	return E_OK;
}

// -----------------------------------------------------------------------
int mx_punchreader_cfg_log(struct mx_unit_proto_t *unit, struct mx_cf_sc_ll *cfg_log)
{
	LOG(D_IO, 10, "MULTIX/punchreader (line:%i): configure logical line", unit->phy_num);
	return E_OK;
}

// -----------------------------------------------------------------------
void mx_punchreader_cmd_attach(struct mx_unit_proto_t *unit, uint16_t addr)
{
	LOG(D_IO, 10, "MULTIX/punchreader (line:%i): attach", unit->log_num);
	unit->attached = 1;
	mx_int(unit->chan, unit->log_num, MX_INT_IDOLI);
}

// -----------------------------------------------------------------------
void mx_punchreader_cmd_detach(struct mx_unit_proto_t *unit, uint16_t addr)
{
	LOG(D_IO, 10, "MULTIX/punchreader (line:%i): detach", unit->log_num);
	unit->attached = 0;
	mx_int(unit->chan, unit->log_num, MX_INT_IODLI);
}

// -----------------------------------------------------------------------
void mx_punchreader_cmd_status(struct mx_unit_proto_t *unit, uint16_t addr)
{
	LOG(D_IO, 10, "MULTIX/punchreader (line:%i): status", unit->log_num);
	mx_int(unit->chan, unit->log_num, MX_INT_ISTRE);
}

// -----------------------------------------------------------------------
void mx_punchreader_cmd_transmit(struct mx_unit_proto_t *unit, uint16_t addr)
{
    // we're transmitting
    pthread_mutex_trylock(&unit->transmit_mutex);

    LOG(D_IO, 1, "MULTIX/punchreader (line:%i): transmit", unit->log_num);
    int ret = E_OK;
    //MEMBw(0, addr+6, cf->ret_len);

    if (ret == E_OK) {
        mx_int(unit->chan, unit->log_num, MX_INT_IETRA);
    } else if (ret == E_MX_CANCEL) {
        mx_int(unit->chan, unit->log_num, MX_INT_ITRAB);
    } else {
        //MEMBw(0, addr+6, cf->ret_status);
        mx_int(unit->chan, unit->log_num, MX_INT_ITRER);
    }

    // done transmitting
    pthread_mutex_unlock(&unit->transmit_mutex);
}

// -----------------------------------------------------------------------
struct mx_punchreader_cf_t * mx_punchreader_cf_t_decode(int addr)
{
	uint16_t data;
	struct mx_punchreader_cf_t *cf = calloc(1, sizeof(void));
	if (!cf) {
		return NULL;
	}

	data = MEMB(0, addr);

	return cf;
}

// -----------------------------------------------------------------------
void mx_punchreader_cf_t_free(struct mx_punchreader_cf_t *cf)
{
	free(cf);
}

// vim: tabstop=4 shiftwidth=4 autoindent
