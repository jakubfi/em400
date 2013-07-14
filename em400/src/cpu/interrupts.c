//  Copyright (c) 2012-2013 Jakub Filipowicz <jakubf@gmail.com>
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

#include <inttypes.h>
#include <pthread.h>

#include "cpu/memory.h"
#include "cpu/registers.h"
#include "cpu/interrupts.h"
#include "io/io.h"
#include "io/chan.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#endif
#include "debugger/log.h"

uint32_t RZ;
uint32_t RP;

pthread_mutex_t int_mutex_rz = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t int_mutex_rp = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t int_cond_rp = PTHREAD_COND_INITIALIZER;

// for extending RM into 32-bit xmask
const uint32_t int_rm2xmask[10] = {
	0b01000000000000000000000000000000,
	0b00100000000000000000000000000000,
	0b00010000000000000000000000000000,
	0b00001000000000000000000000000000,
	0b00000111111100000000000000000000,
	0b00000000000011000000000000000000,
	0b00000000000000110000000000000000,
	0b00000000000000001111110000000000,
	0b00000000000000000000001111110000,
	0b00000000000000000000000000001111
};

// bit masks (to use on SR) for each interrupt
const int int_int2mask[32] = {
	MASK_0,
	MASK_0,
	MASK_1,
	MASK_2,
	MASK_3,
	MASK_4, MASK_4, MASK_4, MASK_4, MASK_4, MASK_4,MASK_4,
	MASK_5, MASK_5,
	MASK_6, MASK_6,
	MASK_7, MASK_7, MASK_7, MASK_7, MASK_7, MASK_7,
	MASK_8, MASK_8, MASK_8, MASK_8, MASK_8, MASK_8,
	MASK_9, MASK_9, MASK_9, MASK_9
};

// -----------------------------------------------------------------------
void int_update_rp()
{
	uint16_t sr = nR(R_SR);
	uint32_t xmask = 0b10000000000000000000000000000000;

	for (int i=0 ; i<10 ; i++) {
		if (sr & (1 << (15-i))) {
			xmask |= int_rm2xmask[i];
		}
	}

	pthread_mutex_lock(&int_mutex_rp);
	RP = RZ & xmask;
	pthread_cond_signal(&int_cond_rp);
	pthread_mutex_unlock(&int_mutex_rp);
}

// -----------------------------------------------------------------------
void int_set(int x)
{
#ifdef WITH_DEBUGGER
	if (x != INT_TIMER) {
		LOG(D_INT, 1, "Set: %lld (%s)", x, log_int_name[x]);
	} else {
		LOG(D_INT, 100, "Set: %lld (%s)", x, log_int_name[x]);
	}
#endif
	pthread_mutex_lock(&int_mutex_rz);
	RZ |= (1 << (31 - x));
	pthread_mutex_unlock(&int_mutex_rz);
	int_update_rp();
}

// -----------------------------------------------------------------------
void int_clear_all()
{
	pthread_mutex_lock(&int_mutex_rz);
	RZ = 0;
	pthread_mutex_unlock(&int_mutex_rz);

	pthread_mutex_lock(&int_mutex_rp);
	RP = 0;
	pthread_mutex_unlock(&int_mutex_rp);
}

// -----------------------------------------------------------------------
void int_clear(int x)
{
	LOG(D_INT, 1, "Clear: %lld (%s)", x, log_int_name[x]);
	pthread_mutex_lock(&int_mutex_rz);
	RZ &= ~((1 << (31 - x)));
	pthread_mutex_unlock(&int_mutex_rz);

	pthread_mutex_lock(&int_mutex_rp);
	RP &= ~((1 << (31 - x)));
	pthread_mutex_unlock(&int_mutex_rp);
}

// -----------------------------------------------------------------------
void int_put_nchan(uint16_t r)
{
	LOG(D_INT, 1, "Set non-channel to: %d", r);
	pthread_mutex_lock(&int_mutex_rz);
	RZ = (RZ & 0b00000000000011111111111111110000) | ((r & 0b1111111111110000) << 16) | (r & 0b0000000000001111);
	pthread_mutex_unlock(&int_mutex_rz);
	int_update_rp();
}

// -----------------------------------------------------------------------
uint16_t int_get_nchan()
{
	uint32_t r = RZ;
	return ((r & 0b11111111111100000000000000000000) >> 16) | (r & 0b00000000000000000000000000001111);
}

// -----------------------------------------------------------------------
void int_serve()
{
	uint32_t rp = RP;

	// no interrupt to serve
	if (!rp) return;
	// not serving after MOD or P
	if (nR(R_MODc) || nR(R_P)) return;

	// find highest interrupt to serve
	int probe = 31;
	while ((probe > 0) && !(rp & (1 << probe))) {
		probe--;
	}

	// this is the interrupt we're going to serve
	int interrupt = 31 - probe;

	LOG(D_INT, 1, "Serve: %d (%s)", interrupt, log_int_name[interrupt]);

	uint16_t int_spec = 0;
	// get interrupt specification if it's from channel
	if ((interrupt >= 12) && (interrupt <= 27)) {
		io_chan[interrupt-12]->cmd(io_chan[interrupt-12], IO_IN, CHAN_CMD_INTSPEC<<8, &int_spec);
	}

	// put system status on stack
	uint16_t SP = nMEMB(0, 97);
	nMEMBw(0, SP, nR(R_IC));
	nMEMBw(0, SP+1, nR(0));
	nMEMBw(0, SP+2, nR(R_SR));
	nMEMBw(0, SP+3, int_spec);

	// clear stuff and get ready to serve
	reg_write(0, 0, 1, 1);
	// mask interrupts with prio <= interrupt
	nRw(R_SR, nR(R_SR) & int_int2mask[interrupt]);
	int_clear(interrupt);
	nRw(R_IC, nMEMB(0, 64+interrupt));
	nMEMBw(0, 97, SP+4);
	SR_Qcb;
#ifdef WITH_DEBUGGER
	dbg_touch_add(&touch_int, 1, 0, interrupt, 0);
#endif
}


// vim: tabstop=4 shiftwidth=4 autoindent
