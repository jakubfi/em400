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
#include <pthread.h>
#include <arpa/inet.h>

#include "debugger/log.h"
#include "errors.h"
#include "cpu/memory.h"

#include "io/multix_winch.h"
#include "e4image.h"

#define UNIT ((struct mx_unit_winch_t *)(unit))

// -----------------------------------------------------------------------
struct mx_unit_proto_t * mx_winch_create(struct cfg_arg_t *args)
{
	char *image_name = NULL;
	int res;
	res = cfg_args_decode(args, "s", &image_name);
	if (res != E_OK) {
		gerr = res;
		return NULL;
	}

	struct e4i_t *winchester = e4i_open(image_name);
	res = E_OK;

	if (!winchester) {
		printf("Error opening image %s: %s\n", image_name, e4i_get_err(e4i_err));
		res = E_IMAGE;
	}

	if (winchester->img_type != E4I_T_HDD) {
		printf("Error opening image %s: wrong image type, expecting hdd\n", image_name);
		res = E_IMAGE;
	}

	if ((winchester->cylinders != 615) || (winchester->heads != 4) || (winchester->spt != 16) || (winchester->block_size != 512)) {
		printf("Error opening image %s: wrong geometry\n", image_name);
		res = E_IMAGE;
	}

	if (res != E_OK) {
		free(image_name);
		if (winchester) {
			e4i_close(winchester);
		}
		gerr = res;
		return NULL;
	}

	eprint("      Winchester: cyl=%i, head=%i, sectors=%i, spt=%i, image=%s\n", winchester->cylinders, winchester->heads, winchester->spt, winchester->block_size, image_name);

	struct mx_unit_proto_t *unit = mx_winch_create_nodev();
	if (!unit) {
		e4i_close(winchester);
		free(image_name);
		gerr = E_ALLOC;
		return NULL;
	}

	free(image_name);
	mx_winch_connect(UNIT, winchester);

	return unit;
}

// -----------------------------------------------------------------------
struct mx_unit_proto_t * mx_winch_create_nodev()
{
	struct mx_unit_winch_t *unit = calloc(1, sizeof(struct mx_unit_winch_t));
	return (struct mx_unit_proto_t *) unit;
}

// -----------------------------------------------------------------------
void mx_winch_connect(struct mx_unit_winch_t *unit, struct e4i_t *winchester)
{
	UNIT->winchester = winchester;
}

// -----------------------------------------------------------------------
void mx_winch_disconnect(struct mx_unit_winch_t *unit)
{
	e4i_close(UNIT->winchester);
	UNIT->winchester = NULL;
}

// -----------------------------------------------------------------------
void mx_winch_shutdown(struct mx_unit_proto_t *unit)
{
	mx_winch_disconnect(UNIT);
	free(UNIT);
}

// -----------------------------------------------------------------------
void mx_winch_reset(struct mx_unit_proto_t *unit)
{
}

// -----------------------------------------------------------------------
int mx_winch_cfg_phy(struct mx_unit_proto_t *unit, struct mx_cf_sc_pl *cfg_phy)
{
	LOG(D_IO, 10, "MULTIX/winchester (line:%i): configure physical line", unit->phy_num);
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
int mx_winch_cfg_log(struct mx_unit_proto_t *unit, struct mx_cf_sc_ll *cfg_log)
{
	LOG(D_IO, 10, "MULTIX/winchester (line:%i): configure logical line", unit->phy_num);
	if (unit && cfg_log && cfg_log->winch) {
		UNIT->winch_type = cfg_log->winch->type;
		UNIT->format_protect = cfg_log->winch->format_protect;
	} else {
		return E_MX_DECODE;
	}
	return E_OK;
}

// -----------------------------------------------------------------------
void mx_winch_cmd_attach(struct mx_unit_proto_t *unit, uint16_t addr)
{
	LOG(D_IO, 10, "MULTIX/winchester (line:%i): attach", unit->log_num);
	unit->attached = 1;
	mx_int(unit->chan, unit->log_num, MX_INT_IDOLI);
}

// -----------------------------------------------------------------------
void mx_winch_cmd_detach(struct mx_unit_proto_t *unit, uint16_t addr)
{
	LOG(D_IO, 10, "MULTIX/winchester (line:%i): detach", unit->log_num);
	unit->attached = 0;
	mx_int(unit->chan, unit->log_num, MX_INT_IODLI);
}

// -----------------------------------------------------------------------
void mx_winch_cmd_status(struct mx_unit_proto_t *unit, uint16_t addr)
{
	LOG(D_IO, 10, "MULTIX/winchester (line:%i): status", unit->log_num);
	mx_int(unit->chan, unit->log_num, MX_INT_ISTRE);
}

// -----------------------------------------------------------------------
int mx_winch_read(struct mx_unit_proto_t *unit, struct mx_winch_cf_t *cf)
{
	int ret = E_OK;

	// first physical cylinder is used by multix for relocated sectors
	int offset = UNIT->winchester->heads * UNIT->winchester->spt;
	uint8_t *buf = malloc(UNIT->winchester->block_size);

	int buf_pos = 0;
	int sector = 0;
	cf->ret_len = 0;

	// transmit data into buffer, sector by sector
	while (cf->ret_len < cf->transmit->len) {

		// still allowed to transmit?
		if (pthread_mutex_trylock(&unit->transmit_mutex) == 0) {
			pthread_mutex_unlock(&unit->transmit_mutex);
			ret = E_MX_CANCEL;
			break;
		}

		LOG(D_IO, 10, "MULTIX/winchester (line:%i): reading sector %i (+offset %i) into buf at pos: 0x%04x", unit->log_num, cf->transmit->sector + sector, offset, cf->transmit->addr + sector * UNIT->winchester->block_size);

		// read whole sector into buffer
		int res = e4i_bread(UNIT->winchester, buf, offset + cf->transmit->sector + sector);

		// sector read OK
		if (res == E_OK) {
			// copy read data into system memory, swapping byte order
			while ((buf_pos < UNIT->winchester->block_size) && (cf->ret_len < cf->transmit->len)) {
				uint16_t *buf16 = (uint16_t*)(buf+buf_pos);
				MEMBw(cf->transmit->nb, cf->transmit->addr + sector * UNIT->winchester->block_size/2 + buf_pos/2, ntohs(*buf16));
				buf_pos += 2;
				cf->ret_len++;
			}
			sector++;
			buf_pos = 0;

		// sector not found or incomplete
		} else if ((res == E4I_E_NO_SECTOR) || (res == E4I_E_READ)) {
			cf->ret_status = MX_WS_ERR | MX_WS_NO_SECTOR;
			ret = E_MX_TRANSMISSION;
			break;

		// shouldn't happen, but let's consider it
		} else {
			cf->ret_status = MX_WS_ERR | MX_WS_REJECTED;
			ret = E_MX_TRANSMISSION;
			break;
		}
	}

	free(buf);

	return ret;
}

// -----------------------------------------------------------------------
void mx_winch_cmd_transmit(struct mx_unit_proto_t *unit, uint16_t addr)
{
	// we're transmitting
	pthread_mutex_trylock(&unit->transmit_mutex);

	LOG(D_IO, 1, "MULTIX/winchester (line:%i): transmit", unit->log_num);

	// disk is not connected
	if (!UNIT->winchester) {
		MEMBw(0, addr+6, MX_WS_ERR | MX_WS_REJECTED);
		mx_int(unit->chan, unit->log_num, MX_INT_ITRER);
		return;
	}

	int ret = E_OK;

	// decode control field
	struct mx_winch_cf_t *cf = mx_winch_cf_t_decode(addr);
	switch (cf->oper) {
		case MX_WINCH_FORMAT_SPARE:
			break;
		case MX_WINCH_FORMAT:
			break;
		case MX_WINCH_READ:
			ret = mx_winch_read(unit, cf);
			break;
		case MX_WINCH_WRITE:
			break;
		case MX_WINCH_PARK:
			// trrrrrrrrrrrr... done.
			ret = E_OK;
			break;
		default:
			// shouldn't happen
			mx_int(unit->chan, unit->log_num, MX_INT_INTRA);
			mx_winch_cf_t_free(cf);
			return;
	}

	MEMBw(0, addr+5, cf->ret_len);
	MEMBw(0, addr+6, cf->ret_status);

	if (ret == E_OK) {
		mx_int(unit->chan, unit->log_num, MX_INT_IETRA);
	} else if (ret == E_MX_CANCEL) {
		mx_int(unit->chan, unit->log_num, MX_INT_ITRAB);
	} else {
		mx_int(unit->chan, unit->log_num, MX_INT_ITRER);
	}

	mx_winch_cf_t_free(cf);

	// done transmitting
	pthread_mutex_unlock(&unit->transmit_mutex);
}

// -----------------------------------------------------------------------
struct mx_winch_cf_t * mx_winch_cf_t_decode(int addr)
{
	uint16_t data;
	struct mx_winch_cf_t *cf = calloc(1, sizeof(struct mx_winch_cf_t));
	if (!cf) {
		return NULL;
	}

	data = MEMB(0, addr);
	cf->oper = (data & 0b0000001100000000) >> 8;
	switch (cf->oper) {
		case MX_WINCH_FORMAT_SPARE:
			cf->format = calloc(1, sizeof(struct mx_winch_cf_format));
			data = MEMB(0, addr+1);
			cf->format->sector_map = data;
			data = MEMB(0, addr+2);
			cf->format->start_sector = data << 16;
			data = MEMB(0, addr+3);
			cf->format->start_sector += data;
			break;
		case MX_WINCH_FORMAT:
			// nothing else
			break;
		case MX_WINCH_READ:
		case MX_WINCH_WRITE:
			cf->transmit = calloc(1, sizeof(struct mx_winch_cf_transmit));
			cf->transmit->ign_crc		= (data & 0b0001000000000000) >> 12;
			cf->transmit->sector_fill	= (data & 0b0000100000000000) >> 11;
			cf->transmit->watch_eof		= (data & 0b0000010000000000) >> 10;
			cf->transmit->cpu			= (data & 0b0000000000010000) >> 4;
			cf->transmit->nb			= (data & 0b0000000000001111);
			data = MEMB(0, addr+1);
			cf->transmit->addr = data;
			data = MEMB(0, addr+2);
			cf->transmit->len = data+1;
			data = MEMB(0, addr+3);
			cf->transmit->sector = data << 16;
			data = MEMB(0, addr+4);
			cf->transmit->sector += data;
			break;
		case MX_WINCH_PARK:
			cf->park = calloc(1, sizeof(struct mx_winch_cf_park));
			data = MEMB(0, addr+4);
			cf->park->cylinder = data;
			break;
		default:
			break;
	}

	return cf;
}

// -----------------------------------------------------------------------
void mx_winch_cf_t_free(struct mx_winch_cf_t *cf)
{
	if (cf->format) free(cf->format);
	if (cf->park) free(cf->park);
	if (cf->transmit) free(cf->transmit);
	free(cf);
}

// vim: tabstop=4 shiftwidth=4 autoindent
