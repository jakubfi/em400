//  Copyright (c) 2013-2014 Jakub Filipowicz <jakubf@gmail.com>
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

#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <mem/mem.h>

#include "io/mx_intr.h"
#include "io/mx_cf.h"
#include "io/mx_proto.h"

#include "io/io.h"

#include "log.h"

#define CHAN ((struct mx_chan *)(chan))
#define LLINE (CHAN->lline[llinen])
#define PLINE (CHAN->pline+plinen)

// -----------------------------------------------------------------------
void mx_cmd_int_requeue(struct chan *chan)
{
	pthread_mutex_lock(&CHAN->intr_mutex);

	struct mx_intr *mxintr = mx_intr_deq(CHAN);
	if (mxintr) {
		mx_intr_enq(CHAN, mxintr);
		// if queue is >1 
		if (CHAN->intr_tail != CHAN->intr_head) {
			mx_intr_report(CHAN);
		}
	}

	pthread_mutex_unlock(&CHAN->intr_mutex);
}

// -----------------------------------------------------------------------
void mx_cmd_intspec(struct chan *chan, uint16_t *r_arg)
{
	pthread_mutex_lock(&CHAN->intr_mutex);

	struct mx_intr *inth = mx_intr_deq(CHAN);

	LOG(L_MX, 2, "MULTIX (ch:%i) command: intspec: (int: %i %s, lline: %i)", chan->num, inth->intr, mx_intr_names[inth->intr], inth->llinen);

	if (inth) {
		*r_arg = (inth->intr << 8) | (inth->llinen);
		free(inth);
	} else {
		*r_arg = MX_INTR_INIEA << 8;
	}

	mx_intr_report(CHAN);

	pthread_mutex_unlock(&CHAN->intr_mutex);
}

// -----------------------------------------------------------------------
void mx_cmd_illegal(struct chan *chan, int llinen, int intr)
{
	LOG(L_MX, 1, "MULTIX (ch:%i, line:%i) command: illegal command, reporting int %d: %s", chan->num, llinen, intr, mx_intr_names[intr]);
	mx_int(CHAN, 0, intr);
}

// -----------------------------------------------------------------------
void mx_cmd_test(struct chan *chan)
{
	LOG(L_MX, 2, "MULTIX (ch:%i) TEST", chan->num);
	usleep(13000);
	mx_int(CHAN, 0, MX_INTR_IWYTE);
}

// -----------------------------------------------------------------------
static int mx_decode_cf_phy(struct chan *chan, int addr, int *offset)
{
	uint16_t data;

	mem_get(0, addr, &data);

	int dir		= (data & 0b1110000000000000) >> 13;
	int used	= (data & 0b0001000000000000) >> 12;
	int type	= (data & 0b0000111100000000) >> 8;
	int count	= (data & 0b0000000000011111) + 1;

	if (used) {
		// check type for correctness
		if ((type < 0) || (type >= MX_PHY_MAX)) {
			return MX_SC_E_DEVTYPE | *offset;
		}

		// check direction for correctness
		if ((type <= MX_PHY_USART_SYNC)
		&& (dir != MX_DIR_OUTPUT)
		&& (dir != MX_DIR_INPUT)
		&& (dir != MX_DIR_HALF_DUPLEX)
		&& (dir != MX_DIR_FULL_DUPLEX)) {
			return MX_SC_E_DIR | *offset;
		}
	}

	for (int plinen=*offset ; plinen<*offset+count ; plinen++) {
		if ((plinen < 0) || (plinen >= MX_LINE_MAX)) {
			return MX_SC_E_NUMLINES;
		}
		PLINE->dir = dir;
		PLINE->used = used;
		PLINE->type = type;
	}

	*offset += count;

	return MX_SC_E_OK;
}

// -----------------------------------------------------------------------
static int mx_decode_cf_log(struct chan *chan, uint16_t addr, int llinen)
{
	int res;
	uint16_t data[4];
	mem_mget(0, addr, data, 4);

	int proto	= (data[0] & 0b1111111100000000) >> 8;
	int plinen	= (data[0] & 0b0000000011111111);

	// make sure physical line is active (configured)
	if (!PLINE->used) {
		return MX_SC_E_PHY_UNUSED | llinen;
	}

	// make sure that no logical line uses this physical line
	if (PLINE->used > 1) {
		return MX_SC_E_PHY_BUSY | llinen;
	}

	LLINE = PLINE;
	res = mx_proto_create(proto, data+1, LLINE);

	if (res != MX_SC_E_OK) {
		return res | llinen;
	}

	LLINE->used = 2;

	return MX_SC_E_OK;
}

// -----------------------------------------------------------------------
int mx_decode_cf_sc(struct chan *chan, int addr)
{
	int res;
	uint16_t data;

	// --- word 0 - header ---
	mem_get(0, addr, &data);
	int pline_desc_count = (data & 0b1111111100000000) >> 8;
	int lline_desc_count = (data & 0b0000000011111111);

	// sanity checks
	if ((pline_desc_count <= 0) || (pline_desc_count > MX_LINE_MAX)) {
		// missing physical line description
		return MX_SC_E_NUMLINES;
	}
	if ((lline_desc_count <= 0) || (lline_desc_count > MX_LINE_MAX)) {
		// missing logical line descroption
		return MX_SC_E_NUMLINES;
	}

	// --- word 1 - return field - used by 'cmd' multix function to report status back

	// --- physical lines, 1 word each ---
	int offset = 0;
	for (int pln=0 ; pln<pline_desc_count ; pln++) {
		// updates offset according to number of plines configured
		res = mx_decode_cf_phy(chan, addr+2+pln, &offset);
		if (res != MX_SC_E_OK) {
			return res;
		}
	}

	// --- logical lines, 4 words each ---
	for (int llinen=0 ; llinen<lline_desc_count ; llinen++) {
		res = mx_decode_cf_log(chan, addr+2+pline_desc_count+(llinen*4), llinen);
		if (res != MX_SC_E_OK) {
			return res;
		}
	}

	return MX_SC_E_OK;
}

// -----------------------------------------------------------------------
void mx_cmd_confset(struct chan *chan, uint16_t addr)
{
	LOG(L_MX, 1, "MULTIX (ch:%i) command: setconf", chan->num);

	uint16_t retf_addr = addr + 1;

	pthread_mutex_lock(&CHAN->conf_mutex);

	if (CHAN->conf_set) {
		pthread_mutex_unlock(&CHAN->conf_mutex);
		LOG(L_MX, 1, "MULTIX setconf: configuration already set");
		mem_put(0, retf_addr, MX_SC_E_CONFSET);
		mx_int(CHAN, 0, MX_INTR_INKON);
	} else {
		// set configuration
	}

	pthread_mutex_unlock(&CHAN->conf_mutex);
}


// vim: tabstop=4 shiftwidth=4 autoindent
