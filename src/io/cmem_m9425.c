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
#include "log.h"
#include "mem/mem.h"
#include "io/cmem.h"
#include "io/cmem_m9425.h"

#define UNIT ((struct cmem_unit_m9425_t *)(unit))

// -----------------------------------------------------------------------
struct cmem_unit_proto_t * cmem_m9425_create(struct cfg_arg_t *args)
{
	char *image_name[2] = { NULL, NULL };
	struct cmem_unit_m9425_t *unit = calloc(1, sizeof(struct cmem_unit_m9425_t));
	int res;

	if (!unit) {
		goto fail;
	}

	if ((image_name[0] || image_name[1]) && !strcmp(image_name[0], image_name[1])) {
		printf("Error opening image: Trying to use the same image for fixed and removable disk");
		res = E_IMAGE;
		goto fail;
	}

	res = cfg_args_decode(args, "ss", &image_name[0], &image_name[1]);
	if (res != E_OK) {
		gerr = res;
		goto fail;
	}

	for (int i=0 ; i<=1 ; i++) {
		UNIT->disk[i] = e4i_open(image_name[i]);

		if (!UNIT->disk[i]) {
			printf("Error opening image %s: %s\n", image_name[i], e4i_get_err(e4i_err));
			res = E_IMAGE;
			goto fail;
		}

		if (UNIT->disk[i]->img_type != E4I_T_HDD) {
			printf("Error opening image %s: wrong image type, expecting hdd\n", image_name[i]);
			res = E_IMAGE;
			goto fail;
		}

		if ((UNIT->disk[i]->cylinders != 203) || (UNIT->disk[i]->heads != 2) || (UNIT->disk[i]->spt != 12) || (UNIT->disk[i]->block_size != 512)) {
			printf("Error opening image %s: wrong geometry\n", image_name[i]);
			res = E_IMAGE;
			goto fail;
		}
		LOG(L_9425, 1, "MERA 9425 (plate %i): cyl=%i, head=%i, sectors=%i, spt=%i, image=%s", i, UNIT->disk[i]->cylinders, UNIT->disk[i]->heads, UNIT->disk[i]->spt, UNIT->disk[i]->block_size, image_name[i]);
		free(image_name[i]);
		image_name[i] = NULL;
	}

	res = pthread_create(&unit->worker, NULL, cmem_m9425_worker, (void*) unit);
	if (res != 0) {
		gerr = E_THREAD;
		goto fail;
	}

	return (struct cmem_unit_proto_t *) unit;

fail:
	for (int i=0 ; i<=1 ; i++) {
		free(image_name[i]);
	}
	cmem_m9425_shutdown((struct cmem_unit_proto_t*) unit);
	return NULL;
}

// -----------------------------------------------------------------------
void cmem_m9425_shutdown(struct cmem_unit_proto_t *unit)
{
	if (unit) {
		for (int i=0 ; i<=1 ; i++) {
			e4i_close(UNIT->disk[i]);
			UNIT->disk[i] = NULL;
		}
		free(UNIT);
	}
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
	uint16_t data[7];
	mem_mget(0, addr, data, 7);

	cf->cf_len			= (data[0] & 0b0000111100000000) >> 8;
	cf->cpu				= (data[0] & 0b0000000000010000) >> 4;
	cf->nb				= (data[0] & 0b0000000000001111);
	cf->oper			= (data[1] & 0b0000011000000000) >> 9;
	cf->len				= data[2];
	cf->ign_wrprotect	= (data[3] & 0b1000000000000000) >> 15;
	cf->ign_defects		= (data[3] & 0b0100000000000000) >> 14;
	cf->ign_key			= (data[3] & 0b0010000000000000) >> 13;
	cf->ign_eof			= (data[3] & 0b0001000000000000) >> 12;
	cf->cyl				= (data[3] & 0b0000000111111111);
	cf->platter			= (data[4] & 0b0000010000000000) >> 10;
	cf->head			= (data[4] & 0b0000000100000000) >> 8;
	cf->sector			= (data[4] & 0b0000000000001111);
	cf->key				= data[5];
	cf->addr			= data[6];

	return E_OK;
}


// vim: tabstop=4 shiftwidth=4 autoindent
