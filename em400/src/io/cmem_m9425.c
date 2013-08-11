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
#include <string.h>
#include <inttypes.h>

#include "errors.h"
#include "cpu/memory.h"
#include "io/cmem.h"
#include "io/cmem_m9425.h"

#define UNIT ((struct cmem_unit_m9425_t *)(unit))

// -----------------------------------------------------------------------
struct cmem_unit_proto_t * cmem_m9425_create(struct cfg_arg_t *args)
{
	char *image_bottom = NULL;
	char *image_top = NULL;
	int res;
	res = cfg_args_decode(args, "ss", &image_bottom, &image_top);
	if (res != E_OK) {
		gerr = res;
		return NULL;
	}

	res = E_OK;

	struct e4i_t *disk_bottom = e4i_open(image_bottom);
	if (!disk_bottom) {
		printf("Error opening image %s: %s\n", image_bottom, e4i_get_err(e4i_err));
		res = E_IMAGE;
	}

	if (disk_bottom->img_type != E4I_T_HDD) {
		printf("Error opening image %s: wrong image type, expecting hdd\n", image_bottom);
		res = E_IMAGE;
	}

	if ((disk_bottom->cylinders != 203) || (disk_bottom->heads != 2) || (disk_bottom->spt != 12) || (disk_bottom->block_size != 512)) {
		printf("Error opening image %s: wrong geometry\n", image_bottom);
		res = E_IMAGE;
	}

	if (!strcmp(image_top, image_bottom)) {
		printf("Error opening image: Trying to use the same image for fixed and removable disk");
		res = E_IMAGE;
	}

	if (res != E_OK) {
		free(image_bottom);
		if (disk_bottom) {
			e4i_close(disk_bottom);
		}
		gerr = res;
		return NULL;
	}

	struct e4i_t *disk_top = e4i_open(image_top);
	if (!disk_top) {
		printf("Error opening image %s: %s\n", image_top, e4i_get_err(e4i_err));
		res = E_IMAGE;
	}
	if (disk_top->img_type != E4I_T_HDC) {
		printf("Error opening image %s: wrong image type, expecting hdc\n", image_top);
		res = E_IMAGE;
	}

	if ((disk_top->cylinders != 203) || (disk_top->heads != 2) || (disk_top->spt != 12) || (disk_top->block_size != 512)) {
		printf("Error opening image %s: wrong geometry\n", image_top);
		res = E_IMAGE;
	}

	if (res != E_OK) {
		free(image_top);
		free(image_bottom);
		e4i_close(disk_bottom);
		if (disk_top) {
			e4i_close(disk_top);
		}
		gerr = res;
		return NULL;
	}

	eprint("      MERA 9425 fixed    : cyl=%i, head=%i, sectors=%i, spt=%i, image=%s\n", disk_bottom->cylinders, disk_bottom->heads, disk_bottom->spt, disk_bottom->block_size, image_bottom);
	eprint("      MERA 9425 removable: cyl=%i, head=%i, sectors=%i, spt=%i, image=%s\n", disk_top->cylinders, disk_top->heads, disk_top->spt, disk_top->block_size, image_top);

	struct cmem_unit_m9425_t *unit = calloc(1, sizeof(struct cmem_unit_m9425_t));
	if (!unit) {
		free(image_top);
		free(image_bottom);
		cmem_m9425_shutdown((struct cmem_unit_proto_t *)unit);
		return NULL;
	}

	UNIT->disk[0] = disk_bottom;
	UNIT->disk[1] = disk_top;

	res = pthread_create(&unit->worker, NULL, cmem_m9425_worker, (void *)unit);
	if (res != 0) {
		free(image_top);
		free(image_bottom);
		cmem_m9425_shutdown((struct cmem_unit_proto_t *)unit);
		gerr = E_THREAD;
		return NULL;
	}

	free(image_top);
	free(image_bottom);

	return (struct cmem_unit_proto_t *) unit;
}

// -----------------------------------------------------------------------
void cmem_m9425_shutdown(struct cmem_unit_proto_t *unit)
{
	e4i_close(UNIT->disk[0]);
	e4i_close(UNIT->disk[1]);
	free(UNIT);
}

// -----------------------------------------------------------------------
void cmem_m9425_reset(struct cmem_unit_proto_t *unit)
{
	// TODO: przerwij transmisję
}

// -----------------------------------------------------------------------
void * cmem_m9425_worker(void *th_id)
{
	struct cmem_unit_proto_t *unit = th_id;
	while (1) {
		switch (UNIT->worker_trans_type) {
			case CMEM_M9425_RD:
				break;
			case CMEM_M9425_RA:
				break;
			case CMEM_M9425_WD:
				break;
			case CMEM_M9425_WA:
				break;
			default:
				break;
		}
	}

	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
int cmem_m9425_cmd(struct cmem_unit_proto_t *unit, int dir, int cmd, uint16_t *r_arg)
{
	if (dir == IO_IN) {
		switch (cmd) {
			case CMEM_M9425_CMD_TEST:
				if (pthread_mutex_trylock(&unit->chan->transmit_mutex) == 0) {
					pthread_mutex_unlock(&unit->chan->transmit_mutex);
					return IO_OK;
				} else {
					return IO_EN;
				}
				break;
			case CMEM_M9425_CMD_TSR:
				// TODO: zwróć status sektora z ostatio czytanego pola adresowego
				*r_arg = 0;
				break;
			case CMEM_M9425_CMD_TCH:
				break;
			default:
				break;
		}
	} else {
		switch (cmd) {
			case CMEM_M9425_CMD_ZER:
				cmem_m9425_reset(unit);
				cmem_int(unit->chan, unit->num, CMEM_M9425_INT_ZER);
				break;
   			case CMEM_M9425_CMD_OTR:
				break;
			case CMEM_M9425_CMD_NTR:
				break;
			case CMEM_M9425_CMD_SEEK:
				// trrrr.. done.
				cmem_int(unit->chan, unit->num, CMEM_M9425_INT_SEEK);
				break;
			case CMEM_M9425_CMD_RTZ:
				// trrrr.. also done.
				cmem_int(unit->chan, unit->num, CMEM_M9425_INT_RTZ);
				break;
			case CMEM_M9425_CMD_SELOFF:
				if (pthread_mutex_trylock(&unit->chan->transmit_mutex) == 0) {
					pthread_mutex_unlock(&unit->chan->transmit_mutex);
					return IO_OK;
				} else {
					return IO_EN;
				}
			case CMEM_M9425_CMD_RES:
				cmem_int(unit->chan, unit->num, CMEM_M9425_INT_RES);
				break;
			default:
				break;
		}
	}
	return IO_OK;
}

// -----------------------------------------------------------------------
int cmem_m9425_decode_cf_t(int addr, struct cmem_m9425_cf_t *cf)
{
	uint16_t data;

	data = MEMB(0, addr);
	cf->cf_len			= (data & 0b0000111100000000) >> 8;
	cf->cpu				= (data & 0b0000000000010000) >> 4;
	cf->nb				= (data & 0b0000000000001111);

	data = MEMB(0, addr+1);
	cf->oper			= (data & 0b0000011000000000) >> 9;

	data = MEMB(0, addr+2);
	cf->len				= data;

	data = MEMB(0, addr+3);
	cf->ign_wrprotect	= (data & 0b1000000000000000) >> 15;
	cf->ign_defects		= (data & 0b0100000000000000) >> 14;
	cf->ign_key			= (data & 0b0010000000000000) >> 13;
	cf->ign_eof			= (data & 0b0001000000000000) >> 12;
	cf->cyl				= (data & 0b0000000111111111);

	data = MEMB(0, addr+4);
	cf->platter			= (data & 0b0000010000000000) >> 10;
	cf->head			= (data & 0b0000000100000000) >> 8;
	cf->sector			= (data & 0b0000000000001111);

	data = MEMB(0, addr+5);
	cf->key				= data;

	data = MEMB(0, addr+6);
	cf->addr			= data;

	return E_OK;
}


// vim: tabstop=4 shiftwidth=4 autoindent
