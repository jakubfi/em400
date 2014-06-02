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

#include "emulog.h"
#include "errors.h"
#include "mem/mem.h"

#include "io/multix_puncher.h"

#define UNIT ((struct mx_unit_puncher_t *)(unit))

// -----------------------------------------------------------------------
struct mx_unit_proto_t * mx_puncher_create(struct cfg_arg_t *args)
{
	eprint("      Tape puncher\n");

	struct mx_unit_proto_t *unit = mx_puncher_create_nodev();
	if (!unit) {
		gerr = E_ALLOC;
		return NULL;
	}

	mx_puncher_connect(UNIT);
	return unit;
}

// -----------------------------------------------------------------------
struct mx_unit_proto_t * mx_puncher_create_nodev()
{
	struct mx_unit_puncher_t *unit = calloc(1, sizeof(struct mx_unit_puncher_t));
	return (struct mx_unit_proto_t *) unit;
}

// -----------------------------------------------------------------------
void mx_puncher_connect(struct mx_unit_puncher_t *unit)
{
}

// -----------------------------------------------------------------------
void mx_puncher_disconnect(struct mx_unit_puncher_t *unit)
{
}

// -----------------------------------------------------------------------
void mx_puncher_shutdown(struct mx_unit_proto_t *unit)
{
	mx_puncher_disconnect(UNIT);
	free(UNIT);
}

// -----------------------------------------------------------------------
void mx_puncher_reset(struct mx_unit_proto_t *unit)
{

}

// -----------------------------------------------------------------------
int mx_puncher_cfg_phy(struct mx_unit_proto_t *unit, struct mx_cf_sc_pl *cfg_phy)
{
	EMULOG(L_PNCH, 10, "MULTIX/puncher (line:%i): configure physical line", unit->phy_num);
	return E_OK;
}

// -----------------------------------------------------------------------
int mx_puncher_cfg_log(struct mx_unit_proto_t *unit, struct mx_cf_sc_ll *cfg_log)
{
	EMULOG(L_PNCH, 10, "MULTIX/puncher (line:%i): configure logical line", unit->phy_num);
	return E_OK;
}

// -----------------------------------------------------------------------
void mx_puncher_cmd_attach(struct mx_unit_proto_t *unit, uint16_t addr)
{
	EMULOG(L_PNCH, 10, "MULTIX/puncher (line:%i): attach", unit->log_num);
	unit->attached = 1;
	mx_int(unit->chan, unit->log_num, MX_INT_IDOLI);
}

// -----------------------------------------------------------------------
void mx_puncher_cmd_detach(struct mx_unit_proto_t *unit, uint16_t addr)
{
	EMULOG(L_PNCH, 10, "MULTIX/puncher (line:%i): detach", unit->log_num);
	unit->attached = 0;
	mx_int(unit->chan, unit->log_num, MX_INT_IODLI);
}

// -----------------------------------------------------------------------
uint16_t mx_puncher_get_status(struct mx_unit_proto_t *unit)
{
	EMULOG(L_PNCH, 10, "MULTIX/puncher (line:%i): status", unit->log_num);
	return 0;
}

// -----------------------------------------------------------------------
void mx_puncher_cmd_transmit(struct mx_unit_proto_t *unit, uint16_t addr)
{
    // we're transmitting
    pthread_mutex_trylock(&unit->transmit_mutex);

    EMULOG(L_PNCH, 1, "MULTIX/puncher (line:%i): transmit", unit->log_num);
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
struct mx_puncher_cf_t * mx_puncher_cf_t_decode(int addr)
{
	struct mx_puncher_cf_t *cf = calloc(1, sizeof(void));
	if (!cf) {
		return NULL;
	}

	return cf;
}

// -----------------------------------------------------------------------
void mx_puncher_cf_t_free(struct mx_puncher_cf_t *cf)
{
	free(cf);
}

// vim: tabstop=4 shiftwidth=4 autoindent
