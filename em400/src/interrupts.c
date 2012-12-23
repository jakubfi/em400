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

uint32_t RZ;
pthread_mutex_t int_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t int_cond = PTHREAD_COND_INITIALIZER;

// bit masks (to use on SR) for each interrupt
int int_rz2rm[32] = {
-1, // NMI, no mask for that
RM_0,
RM_1,
RM_2,
RM_3,
RM_4, RM_4, RM_4, RM_4, RM_4, RM_4,RM_4,
RM_5, RM_5,
RM_6, RM_6,
RM_7, RM_7, RM_7, RM_7, RM_7, RM_7,
RM_8, RM_8, RM_8, RM_8, RM_8, RM_8,
RM_9, RM_9, RM_9, RM_9 };

// bit masks (to use on SR) for each interrupt
int int_rz2mask[32] = {
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
MASK_9, MASK_9, MASK_9, MASK_9 };


// -----------------------------------------------------------------------
void int_set(uint32_t x)
{
	pthread_mutex_lock(&int_mutex);
	RZ |= x;
	pthread_cond_signal(&int_cond);
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
void int_clear(uint32_t x)
{
	pthread_mutex_lock(&int_mutex);
	RZ &= ~(x);
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
void int_put_nchan(uint16_t r)
{
	pthread_mutex_lock(&int_mutex);
	RZ = (RZ & 0b00000000000011111111111111110000) | ((r & 0b1111111111110000) << 16) | (r & 0b0000000000001111);
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
int int_is_masked(int i)
{
	if (i) {
		return !(R(R_SR) & int_rz2rm[i]);
	} else {
		return 0;
	}
}

// -----------------------------------------------------------------------
void int_mask(int i)
{
	Rw(R_SR, R(R_SR) & int_rz2mask[i]);
}

// -----------------------------------------------------------------------
void int_serve()
{
	uint32_t rz = RZ;

	// no interrupt to serve
	if (!rz) return;

	// find highest interrupt to serve
	int probe = 31;
	while (probe && !(rz & (1<<probe))) {
		probe--;
	}

	// no interrupt to serve
	if (!probe) return;

	// this is the interrupt we're going to serve
	int interrupt = 31 - probe;

	// interrupt is masked, nothing to do
	if (int_is_masked(interrupt)) return;

	// put system status on stack
	uint16_t SP = nMEMB(0, 97);
	nMEMBw(0, SP, R(R_IC));
	nMEMBw(0, SP+1, R(0));
	nMEMBw(0, SP+2, R(R_SR));
	nMEMBw(0, SP+3, 0); // TODO: 0 lub specyfikacja przerwania (jeśli z kanału)
	Rw(0, 0);
	int_mask(interrupt);
	int_clear(interrupt);
	Rw(R_IC, nMEMB(0, 64+interrupt));
	nMEMBw(0, 97, SP+4);
	SR_Qcb;
}


// vim: tabstop=4
