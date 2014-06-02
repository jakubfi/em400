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

#include "errors.h"
#include "mem/mem.h"

#include "io/multix_winch.h"
#include "io/e4image.h"

#include "utils.h"

#ifdef WITH_DEBUGGER
#include "debugger/decode.h"
#endif

#include "emulog.h"

#define UNIT ((struct mx_unit_winch_t *)(unit))

// -----------------------------------------------------------------------
struct mx_unit_proto_t * mx_winch_create(struct cfg_arg_t *args)
{
	char *image_name = NULL;
	struct e4i_t *winchester = NULL;
	struct mx_unit_proto_t *unit = NULL;
	int res;

	unit = mx_winch_create_nodev();
	if (!unit) {
		gerr = E_ALLOC;
		goto fail;
	}

	res = cfg_args_decode(args, "s", &image_name);
	if (res != E_OK) {
		gerr = res;
		goto fail;
	}

	winchester = e4i_open(image_name);
	mx_winch_connect(UNIT, winchester);

	if (!winchester) {
		printf("Error opening image %s: %s\n", image_name, e4i_get_err(e4i_err));
		gerr = E_IMAGE;
		goto fail;
	}

	if (winchester->img_type != E4I_T_HDD) {
		printf("Error opening image %s: wrong image type, expecting hdd\n", image_name);
		gerr = E_IMAGE;
		goto fail;
	}

	if ((winchester->cylinders != 615)
	|| (winchester->heads != 4)
	|| (winchester->spt != 16)
	|| (winchester->block_size != 512)) {
		printf("Error opening image %s: wrong geometry\n", image_name);
		gerr = E_IMAGE;
		goto fail;
	}

	eprint("      Winchester: cyl=%i, head=%i, sectors=%i, spt=%i, image=%s\n", winchester->cylinders, winchester->heads, winchester->spt, winchester->block_size, image_name);

	free(image_name);
	image_name = NULL;
	return unit;

fail:
	free(image_name);
	mx_winch_shutdown(unit);
	return NULL;
}

// -----------------------------------------------------------------------
struct mx_unit_proto_t * mx_winch_create_nodev()
{
	struct mx_unit_winch_t *unit = calloc(1, sizeof(struct mx_unit_winch_t));
	return (struct mx_unit_proto_t*) unit;
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
	if (unit) {
		mx_winch_disconnect(UNIT);
		free(UNIT);
	}
}

// -----------------------------------------------------------------------
void mx_winch_reset(struct mx_unit_proto_t *unit)
{
}

// -----------------------------------------------------------------------
int mx_winch_cfg_phy(struct mx_unit_proto_t *unit, struct mx_cf_sc_pl *cfg_phy)
{
	EMULOG(L_WNCH, 10, "MULTIX/winchester (log:%i, phy:%i): configure physical line", unit->log_num, unit->phy_num);
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
	EMULOG(L_WNCH, 10, "MULTIX/winchester (log:%i, phy:%i): configure logical line", unit->log_num, unit->phy_num);
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
	EMULOG(L_WNCH, 10, "MULTIX/winchester (log:%i, phy: %i): attach", unit->log_num, unit->phy_num);
	unit->attached = 1;
	mx_int(unit->chan, unit->log_num, MX_INT_IDOLI);
}

// -----------------------------------------------------------------------
void mx_winch_cmd_detach(struct mx_unit_proto_t *unit, uint16_t addr)
{
	EMULOG(L_WNCH, 10, "MULTIX/winchester (log:%i, phy:%i): detach", unit->log_num, unit->phy_num);
	unit->attached = 0;
	mx_int(unit->chan, unit->log_num, MX_INT_IODLI);
}

// -----------------------------------------------------------------------
uint16_t mx_winch_get_status(struct mx_unit_proto_t *unit)
{
	EMULOG(L_WNCH, 10, "MULTIX/winchester (log:%i, phy:%i): status", unit->log_num, unit->phy_num);
	return 0;
}

// -----------------------------------------------------------------------
int mx_winch_read(struct mx_unit_proto_t *unit, struct mx_winch_cf_t *cf)
{
	int ret = E_OK;
	cf->ret_len = 0;

	// first physical cylinder is used by multix for relocated sectors
	int offset = UNIT->winchester->heads * UNIT->winchester->spt;
	uint8_t *buf = malloc(UNIT->winchester->block_size);

	if (!buf) {
		cf->ret_status = MX_WS_ERR | MX_WS_REJECTED;
		return E_MX_TRANSMISSION;
	}

	int buf_pos = 0;
	int sector = 0;

	// transmit data into buffer, sector by sector
	while (cf->ret_len < cf->transmit->len) {

		// check if we are still allowed to transmit
		if (pthread_mutex_trylock(&unit->transmit_mutex) == 0) {
			pthread_mutex_unlock(&unit->transmit_mutex);
			ret = E_MX_CANCEL;
			break;
		}

		EMULOG(L_WNCH, 10, "MULTIX/winchester (log:%i, phy:%i): read sector %i (+%i) -> %i : 0x%04x", unit->log_num, unit->phy_num, cf->transmit->sector + sector, offset, cf->transmit->nb, cf->transmit->addr + sector * UNIT->winchester->block_size/2);

		// read whole sector into buffer
		int res = e4i_bread(UNIT->winchester, buf, offset + cf->transmit->sector + sector);

		// sector read OK
		if (res == E4I_E_OK) {
			// copy read data into system memory, swapping byte order
			while ((buf_pos < UNIT->winchester->block_size) && (cf->ret_len < cf->transmit->len)) {
				uint16_t *buf16 = (uint16_t*)(buf+buf_pos);
				mem_put(cf->transmit->nb, cf->transmit->addr + sector * UNIT->winchester->block_size/2 + buf_pos/2, ntohs(*buf16));
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
int mx_winch_write(struct mx_unit_proto_t *unit, struct mx_winch_cf_t *cf)
{
	int ret = E_OK;
	uint16_t data;
	int i;
	int res;
	int sector = 0;

	cf->ret_len = 0;

	// first physical cylinder is used by multix for relocated sectors
	int offset = UNIT->winchester->heads * UNIT->winchester->spt;
	uint8_t *buf = malloc(cf->transmit->len*2);

	if (!buf) {
		cf->ret_status = MX_WS_ERR | MX_WS_REJECTED;
		return E_MX_TRANSMISSION;
	}

	// fill buffer with data to write
	for (i=0 ; i<cf->transmit->len ; i++) {
		mem_get(cf->transmit->nb, cf->transmit->addr + i, &data);
		buf[i*2] = data>>8;
		buf[i*2+1] = data&255;
	}

	// write sectors
	while (cf->ret_len < cf->transmit->len) {
		// check if we are still allowed to transmit
		if (pthread_mutex_trylock(&unit->transmit_mutex) == 0) {
			pthread_mutex_unlock(&unit->transmit_mutex);
			ret = E_MX_CANCEL;
			break;
		}

		int transmit = cf->transmit->len - cf->ret_len;
		if (transmit > UNIT->winchester->block_size/2) {
			transmit = UNIT->winchester->block_size/2;
		}

		EMULOG(L_WNCH, 10, "MULTIX/winchester (log:%i, phy:%i): write sector %i (+%i) <- %d : 0x%04x, %i words", unit->log_num, unit->phy_num, cf->transmit->sector + sector, offset, cf->transmit->nb, cf->transmit->addr + cf->ret_len, transmit);

		res = e4i_bwrite(UNIT->winchester, buf+cf->ret_len*2, offset + cf->transmit->sector + sector, transmit*2);
		//res = E_OK;

		// write OK
		if (res == E4I_E_OK) {
			sector++;
			cf->ret_len += transmit;
		// sector not found or incomplete
		} else if ((res == E4I_E_NO_SECTOR) || (res == E4I_E_WRITE)) {
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
	uint16_t addr_len = addr + 5;
	uint16_t addr_status = addr + 6;
	int ret = E_OK;

	EMULOG(L_WNCH, 1, "MULTIX/winchester (log:%i, phy:%i): transmit", unit->log_num, unit->phy_num);

	// disk is not connected
	if (!UNIT->winchester) {
		mem_put(0, addr_status, MX_WS_ERR | MX_WS_REJECTED);
		mx_int(unit->chan, unit->log_num, MX_INT_ITRER);
		return;
	}

	// decode control field
	struct mx_winch_cf_t *cf = mx_winch_cf_t_decode(addr);
	if (!cf) {
		mx_int(unit->chan, unit->log_num, MX_INT_INTRA);
		return;
	}

#ifdef WITH_EMULOG
	char *details;
#ifdef WITH_DEBUGGER
	details = decode_mxpst_winch(0, addr, 0);
#else
	details = malloc(128);
	sprintf(details, "[details missing]");
#endif
	emulog_splitlog(L_WNCH, 50, details);
	free(details);
#endif

	switch (cf->oper) {
		case MX_WINCH_FORMAT_SPARE:
			break;
		case MX_WINCH_FORMAT:
			break;
		case MX_WINCH_READ:
			ret = mx_winch_read(unit, cf);
			break;
		case MX_WINCH_WRITE:
			ret = mx_winch_write(unit, cf);
			break;
		case MX_WINCH_PARK:
			// trrrrrrrrrrrr... done.
			break;
		default: // shouldn't happen, set 'transmit rejected' interrupt
			mx_int(unit->chan, unit->log_num, MX_INT_INTRA);
			mx_winch_cf_t_free(cf);
			return;
	}

	mem_put(0, addr_len, cf->ret_len);
	mem_put(0, addr_status, cf->ret_status);

#ifdef WITH_EMULOG
	char *status = int2binf("........ ........", cf->ret_status, 16);
	EMULOG(L_WNCH, 10, "MULTIX/winchester (log:%i, phy:%i): transmit done, status: %s, transmitted %i words", unit->log_num, unit->phy_num, status, cf->ret_len);
	free(status);
#endif

	if (ret == E_OK) { // transmission finished OK
		mx_int(unit->chan, unit->log_num, MX_INT_IETRA);
	} else if (ret == E_MX_CANCEL) { // transmission cancelled
		mx_int(unit->chan, unit->log_num, MX_INT_ITRAB);
	} else { // transmission finished with error
		mx_int(unit->chan, unit->log_num, MX_INT_ITRER);
	}

	mx_winch_cf_t_free(cf);
}

// -----------------------------------------------------------------------
struct mx_winch_cf_t * mx_winch_cf_t_decode(int addr)
{
	uint16_t data[5];
	struct mx_winch_cf_t *cf = calloc(1, sizeof(struct mx_winch_cf_t));

	if (!cf) {
		goto fail;
	}

	mem_mget(0, addr, data, 5);

	cf->oper = (data[0] & 0b0000001100000000) >> 8;

	switch (cf->oper) {
		case MX_WINCH_FORMAT_SPARE:
			cf->format = calloc(1, sizeof(struct mx_winch_cf_format));
			if (!cf->format) {
				goto fail;
			}
			cf->format->sector_map = data[1];
			cf->format->start_sector = data[2] << 16;
			cf->format->start_sector += data[3];
			break;
		case MX_WINCH_FORMAT:
			break;
		case MX_WINCH_READ:
		case MX_WINCH_WRITE:
			cf->transmit = calloc(1, sizeof(struct mx_winch_cf_transmit));
			if (!cf->transmit) {
				goto fail;
			}
			cf->transmit->ign_crc		= (data[0] & 0b0001000000000000) >> 12;
			cf->transmit->sector_fill	= (data[0] & 0b0000100000000000) >> 11;
			cf->transmit->watch_eof		= (data[0] & 0b0000010000000000) >> 10;
			cf->transmit->cpu			= (data[0] & 0b0000000000010000) >> 4;
			cf->transmit->nb			= (data[0] & 0b0000000000001111);
			cf->transmit->addr = data[1];
			cf->transmit->len = data[2]+1;
			cf->transmit->sector = (data[3] & 255) << 16;
			cf->transmit->sector += data[4];
			break;
		case MX_WINCH_PARK:
			cf->park = calloc(1, sizeof(struct mx_winch_cf_park));
			if (!cf->park) {
				goto fail;
			}
			cf->park->cylinder = data[4];
			break;
		default:
			goto fail;
	}

	return cf;

fail:
	mx_winch_cf_t_free(cf);
	return NULL;
}

// -----------------------------------------------------------------------
void mx_winch_cf_t_free(struct mx_winch_cf_t *cf)
{
	free(cf->format);
	free(cf->park);
	free(cf->transmit);
	free(cf);
}

// vim: tabstop=4 shiftwidth=4 autoindent
