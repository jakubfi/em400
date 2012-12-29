//  Copyright (c) 2012 Jakub Filipowicz <jakubf@gmail.com>
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

#include "memory.h"
#include "registers.h"
#include "interrupts.h"
#include "io.h"

volatile uint32_t RZ;
volatile uint32_t RP;
uint32_t xmask;

pthread_mutex_t int_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t int_cond = PTHREAD_COND_INITIALIZER;

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
	xmask = 0b10000000000000000000000000000000;
	for (int i=0 ; i<10 ; i++) {
		if (nR(R_SR) & (1<<(15-i))) {
			xmask |= int_rm2xmask[i];
		}
	}

	RP = RZ & xmask;
}

// -----------------------------------------------------------------------
void int_set(uint32_t x)
{
	pthread_mutex_lock(&int_mutex);
	RZ |= x;
	int_update_rp();
	pthread_cond_signal(&int_cond);
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
void int_clear(uint32_t x)
{
	pthread_mutex_lock(&int_mutex);
	RZ &= ~(x);
	RP &= ~(x);
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
void int_put_nchan(uint16_t r)
{
	pthread_mutex_lock(&int_mutex);
	RZ = (RZ & 0b00000000000011111111111111110000) | ((r & 0b1111111111110000) << 16) | (r & 0b0000000000001111);
	int_update_rp();
	pthread_cond_signal(&int_cond);
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
uint16_t int_get_nchan()
{
	uint16_t r = RZ;
	return ((r & 0b11111111111100000000000000000000) >> 16) | (r & 0b00000000000000000000000000001111);
}

// -----------------------------------------------------------------------
void int_mask_below(int i)
{
	nRw(R_SR, nR(R_SR) & int_int2mask[i]);
}

// -----------------------------------------------------------------------
void int_serve()
{
	uint32_t rp = RP;

	// no interrupt to serve
	if (!rp) return;

	// do not serve interrupts when P is set or previous instruction was MD
	if (nR(R_P) || nR(R_MODc)) return;

	// find highest interrupt to serve
	int probe = 31;
	while ((probe > 0) && !(rp & (1 << probe))) {
		probe--;
	}

	// no interrupt to serve
	if (!probe) return;

	// this is the interrupt we're going to serve
	int interrupt = 31 - probe;

	int int_spec = 0;
	// get interrupt specification it it's from channel
	if ((interrupt >= 12) && (interrupt <= 27)) {
		int_spec = io_get_int_spec(interrupt);
	}

	// put system status on stack
	uint16_t SP = nMEMB(0, 97);
	nMEMBw(0, SP, nR(R_IC));
	nMEMBw(0, SP+1, nR(0));
	nMEMBw(0, SP+2, nR(R_SR));
	nMEMBw(0, SP+3, int_spec);

	// clear stuff and get ready to serve
	nRw(0, 0);
	int_mask_below(interrupt);
	int_clear(interrupt);
	nRw(R_IC, nMEMB(0, 64+interrupt));
	nMEMBw(0, 97, SP+4);
	SR_Qcb;
}


// vim: tabstop=4
