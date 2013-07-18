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
#include "io/devices/rawdisk.h"

#define UNIT ((struct mx_unit_winch_t *)(unit))

// -----------------------------------------------------------------------
struct mx_unit_proto_t * mx_winch_create(struct cfg_arg_t *args)
{
	int cyl, head, sect, ssize;
	char *image_name = NULL;
	int res;
	res = cfg_args_decode(args, "iiiis", &cyl, &head, &sect, &ssize, &image_name);
	if (res != E_OK) {
		gerr = res;
		return NULL;
	}

	eprint("      Winchester: cyl=%i, head=%i, sectors=%i, spt=%i, image=%s\n", cyl, head, sect, ssize, image_name);

	struct rawdisk_t *winchester = rawdisk_create(cyl, head, sect, ssize, image_name);
	free(image_name);
	if (!winchester) {
		return NULL;
	}

	struct mx_unit_proto_t *unit = mx_winch_create_nodev();
	if (!unit) {
		rawdisk_shutdown(winchester);
		gerr = E_ALLOC;
		return NULL;
	}

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
void mx_winch_connect(struct mx_unit_winch_t *unit, struct rawdisk_t *winchester)
{
	UNIT->winchester = winchester;
}

// -----------------------------------------------------------------------
void mx_winch_disconnect(struct mx_unit_winch_t *unit)
{
	rawdisk_shutdown(UNIT->winchester);
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
	// first cylinder is used for relocated sectors
	int offset = UNIT->winchester->heads * UNIT->winchester->sectors;

	int buffer_size = ((cf->transmit->len / UNIT->winchester->sector_size)+1) * UNIT->winchester->sector_size;
	uint8_t *buf = malloc(buffer_size);
	int res;
	int pos;

	// transmit data into buffer, sector by sector
	pos = 0;
	int sector = cf->transmit->sector;
	while (pos < cf->transmit->len) {
		LOG(D_IO, 10, "MULTIX/winchester (line:%i): reading sector %i (+offset %i) into buf at pos: %i", unit->log_num, sector, offset, pos);
		res = rawdisk_read_sector_l(UNIT->winchester, buf+pos, offset+sector);
		pos += UNIT->winchester->sector_size;
		sector++;
	}

	LOG(D_IO, 10, "MULTIX/winchester (line:%i): copying %i words to %i:%i", unit->log_num, cf->transmit->len, cf->transmit->nb, cf->transmit->addr);
	// copy buffer to memory, swapping byte order
	pos = 0;
	while (pos < cf->transmit->len) {
		uint16_t data = ntohs(*((uint16_t*)(buf+(pos*2))));
		MEMBw(cf->transmit->nb, cf->transmit->addr + pos, data);
		pos++;
	}
	free(buf);

	return E_OK;
}

// -----------------------------------------------------------------------
void mx_winch_cmd_transmit(struct mx_unit_proto_t *unit, uint16_t addr)
{
	LOG(D_IO, 1, "MULTIX/winchester (line:%i): transmit", unit->log_num);

	// disk is not connected
	if (!UNIT->winchester) {
		MEMBw(0, addr+6, 0);
		MEMBw(0, addr+6, MX_WS_ERR | MX_WS_REJECTED);
		mx_int(unit->chan, unit->log_num, MX_INT_ITRER);
		return;
	}

	int res = E_OK;

	// decode control field
	struct mx_winch_cf_t *cf = mx_winch_cf_t_decode(addr);
	switch (cf->oper) {
		case MX_WINCH_FORMAT_SPARE:
			break;
		case MX_WINCH_FORMAT:
			break;
		case MX_WINCH_READ:
			res = mx_winch_read(unit, cf);
			break;
		case MX_WINCH_WRITE:
			break;
		case MX_WINCH_PARK:
			break;
		default:
			mx_int(unit->chan, unit->log_num, MX_INT_INTRA);
			mx_winch_cf_t_free(cf);
			return;
	}

	MEMBw(0, addr+6, cf->ret_len);

	if (res != E_OK) {
		MEMBw(0, addr+6, cf->ret_status);
		mx_int(unit->chan, unit->log_num, MX_INT_ITRER);
	} else {
		mx_int(unit->chan, unit->log_num, MX_INT_IETRA);
	}

	mx_winch_cf_t_free(cf);
}

// -----------------------------------------------------------------------
void mx_winch_cmd_break(struct mx_unit_proto_t *unit, uint16_t addr)
{
	// break while not transmitting
	mx_int(unit->chan, unit->log_num, MX_INT_INABT);
}

// -----------------------------------------------------------------------
void * mx_winch_worker(void *th_id)
{
	struct mx_unit_proto_t *unit = th_id;
	while (1) {
		// wait for command
		pthread_mutex_lock(&unit->worker_mutex);
		while (unit->worker_cmd <= 0) {
			LOG(D_IO, 10, "MULTIX/winchester (line:%i): worker waiting for job...", unit->log_num);
			pthread_cond_wait(&unit->worker_cond, &unit->worker_mutex);
		}

		// do the work
		switch (unit->worker_dir<<7 | unit->worker_cmd) {
			case IO_OU<<7 | MX_LCMD_ATTACH:
				mx_winch_cmd_attach(unit, unit->worker_addr);
				break;
			case IO_IN<<7 | MX_LCMD_DETACH:
				mx_winch_cmd_detach(unit, unit->worker_addr);
				break;
			case IO_OU<<7 | MX_LCMD_STATUS:
				mx_winch_cmd_status(unit, unit->worker_addr);
				break;
			case IO_OU<<7 | MX_LCMD_TRANSMIT:
				mx_winch_cmd_transmit(unit, unit->worker_addr);
				break;
			case IO_IN<<7 | MX_LCMD_BREAK:
				mx_winch_cmd_break(unit, unit->worker_addr);
				break;
			default:
				break;
		}

		unit->worker_cmd = 0;
		LOG(D_IO, 10, "MULTIX/winchester (line:%i): worker done", unit->log_num);
		pthread_mutex_unlock(&unit->worker_mutex);
	}

	pthread_exit(NULL);
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
