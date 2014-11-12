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

#include "log.h"
#include "errors.h"
#include "mem/mem.h"

#include "io/multix_floppy.h"
#include "io/e4image.h"

#define UNIT ((struct mx_unit_floppy_t *)(unit))

// -----------------------------------------------------------------------
struct mx_unit_proto_t * mx_floppy_create(struct cfg_arg *args)
{
	char *image_name = NULL;
	struct e4i_t *floppy = NULL;
	struct mx_unit_proto_t *unit = NULL;
	int res;

	unit = mx_floppy_create_nodev();
	if (!unit) {
		gerr = E_ALLOC;
		goto fail;
	}

	res = cfg_args_decode(args, "s", &image_name);
	if (res != E_OK) {
		gerr = res;
		goto fail;
	}

	floppy = e4i_open(image_name);
	mx_floppy_connect(UNIT, floppy);

	if (!floppy) {
		printf("Error opening image %s: %s\n", image_name, e4i_get_err(e4i_err));
		res = E_IMAGE;
		goto fail;
	}

	if (floppy->img_type != E4I_T_FLOPPY) {
		printf("Error opening image %s: wrong image type, expecting floppy\n", image_name);
		res = E_IMAGE;
		goto fail;
	}

	if ((floppy->cylinders != 80)
	|| (floppy->heads != 2)
	|| (floppy->spt != 15)
	|| (floppy->block_size != 512)) {
		printf("Error opening image %s: wrong geometry\n", image_name);
		res = E_IMAGE;
		goto fail;
	}

	LOG(L_FLOP, 1, "Floppy: cyl=%i, head=%i, sectors=%i, spt=%i, image=%s", floppy->cylinders, floppy->heads, floppy->spt, floppy->block_size, image_name);

	free(image_name);
	image_name = NULL;
	return unit;

fail:
	free(image_name);
	mx_floppy_shutdown(unit);
	return NULL;
}

// -----------------------------------------------------------------------
struct mx_unit_proto_t * mx_floppy_create_nodev()
{
	struct mx_unit_floppy_t *unit = calloc(1, sizeof(struct mx_unit_floppy_t));
	return (struct mx_unit_proto_t *) unit;
}

// -----------------------------------------------------------------------
void mx_floppy_connect(struct mx_unit_floppy_t *unit, struct e4i_t *floppy)
{
	UNIT->floppy = floppy;
}

// -----------------------------------------------------------------------
void mx_floppy_disconnect(struct mx_unit_floppy_t *unit)
{
	e4i_close(UNIT->floppy);
	UNIT->floppy = NULL;
}

// -----------------------------------------------------------------------
void mx_floppy_shutdown(struct mx_unit_proto_t *unit)
{
	if (unit) {
		mx_floppy_disconnect(UNIT);
		free(UNIT);
	}
}

// -----------------------------------------------------------------------
void mx_floppy_reset(struct mx_unit_proto_t *unit)
{

}

// -----------------------------------------------------------------------
int mx_floppy_cfg_phy(struct mx_unit_proto_t *unit, struct mx_cf_sc_pl *cfg_phy)
{
	LOG(L_FLOP, 2, "MULTIX/floppy (line:%i): configure physical line", unit->phy_num);
	if (unit && cfg_phy) {
		unit->dir = cfg_phy->dir;
		unit->used = 1;
		unit->type = cfg_phy->type;
	} else {
		return E_MX_DECODE;
	}
	return E_OK;
}

// -----------------------------------------------------------------------
int mx_floppy_cfg_log(struct mx_unit_proto_t *unit, struct mx_cf_sc_ll *cfg_log)
{
	LOG(L_FLOP, 2, "MULTIX/floppy (line:%i): configure logical line", unit->phy_num);
	if (unit && cfg_log && cfg_log->floppy) {
		UNIT->floppy_type = cfg_log->floppy->type;
		UNIT->format_protect = cfg_log->floppy->format_protect;
	} else {
		return E_MX_DECODE;
	}
	return E_OK;
}

// -----------------------------------------------------------------------
void mx_floppy_cmd_attach(struct mx_unit_proto_t *unit, uint16_t addr)
{
	LOG(L_FLOP, 2, "MULTIX/floppy (line:%i): attach", unit->log_num);
	unit->attached = 1;
	mx_int(unit->chan, unit->log_num, MX_INT_IDOLI);
}

// -----------------------------------------------------------------------
void mx_floppy_cmd_detach(struct mx_unit_proto_t *unit, uint16_t addr)
{
	LOG(L_FLOP, 2, "MULTIX/floppy (line:%i): detach", unit->log_num);
	unit->attached = 0;
	mx_int(unit->chan, unit->log_num, MX_INT_IODLI);
}

// -----------------------------------------------------------------------
uint16_t mx_floppy_get_status(struct mx_unit_proto_t *unit)
{
	LOG(L_FLOP, 2, "MULTIX/floppy (line:%i): status", unit->log_num);
	return 0;
}

// -----------------------------------------------------------------------
void mx_floppy_cmd_transmit(struct mx_unit_proto_t *unit, uint16_t addr)
{
    LOG(L_FLOP, 1, "MULTIX/floppy (line:%i): transmit", unit->log_num);
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
}

// -----------------------------------------------------------------------
struct mx_floppy_cf_t * mx_floppy_cf_t_decode(int addr)
{
	uint16_t data[4];
	struct mx_floppy_cf_t *cf = calloc(1, sizeof(struct mx_floppy_cf_t));
	if (!cf) {
		goto fail;
	}

	mem_mget(0, addr, data, 4);

	cf->oper = (data[0] & 0b0000011100000000) >> 8;
	switch (cf->oper) {
		case MX_FLOPPY_FORMAT:
			cf->format = calloc(1, sizeof(struct mx_floppy_cf_format));
			if (!cf->format) {
				goto fail;
			}
			cf->format->start_sector = data[3];
			break;
		case MX_FLOPPY_READ:
		case MX_FLOPPY_WRITE:
			cf->transmit = calloc(1, sizeof(struct mx_floppy_cf_transmit));
			if (!cf->transmit) {
				goto fail;
			}
			cf->transmit->ign_crc		= (data[0] & 0b0001000000000000) >> 12;
			cf->transmit->cpu			= (data[0] & 0b0000000000010000) >> 4;
			cf->transmit->nb			= (data[0] & 0b0000000000001111);
			cf->transmit->addr = data[1];
			cf->transmit->len = data[2]+1;
			cf->transmit->sector = data[3] << 16;
			break;
		case MX_FLOPPY_BAD_SECTOR:
			cf->bad_sector = calloc(1, sizeof(struct mx_floppy_cf_bad_sector));
			if (!cf->bad_sector) {
				goto fail;
			}
			cf->bad_sector->sector = data[3];
			break;
		default:
			goto fail;
	}

	return cf;

fail:
	mx_floppy_cf_t_free(cf);
	return NULL;
}

// -----------------------------------------------------------------------
void mx_floppy_cf_t_free(struct mx_floppy_cf_t *cf)
{
	free(cf->format);
	free(cf->transmit);
	free(cf->bad_sector);
	free(cf);
}

// vim: tabstop=4 shiftwidth=4 autoindent
