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

#include "cpu/cpu.h"
#include "cpu/registers.h"
#include "cpu/memory.h"
#include "cpu/interrupts.h"
#include "io/io.h"
#include "io/chan.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#endif
#include "debugger/log.h"

uint32_t RZ;
uint32_t RP;

int int_timer = INT_TIMER;
int int_extra = INT_EXTRA;

pthread_spinlock_t int_ready;
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

// bit masks (to use on SR) for each interrupt (always masks Q)
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
void int_wait()
{
	pthread_mutex_lock(&int_mutex);
	while (!RP) {
		pthread_cond_wait(&int_cond, &int_mutex);
	}
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
void int_update_rp()
{
	int i;
	uint16_t sr = regs[R_SR];
	uint32_t xmask = 0b10000000000000000000000000000000;

	for (i=0 ; i<10 ; i++) {
		if (sr & (1 << (15-i))) {
			xmask |= int_rm2xmask[i];
		}
	}

	pthread_mutex_lock(&int_mutex);
	RP = RZ & xmask;
	if (RP) pthread_spin_unlock(&int_ready);
	pthread_cond_signal(&int_cond);
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
void int_set(int x)
{
#ifdef WITH_DEBUGGER
	if (x != int_timer) {
		LOG(L_INT, 20, "Set: %lld (%s)", x, log_int_name[x]);
	} else {
		LOG(L_INT, 100, "Set: %lld (%s)", x, log_int_name[x]);
	}
#endif
	pthread_mutex_lock(&int_mutex);
	RZ |= (1 << (31 - x));
	pthread_mutex_unlock(&int_mutex);
	int_update_rp();
}

// -----------------------------------------------------------------------
void int_clear_all()
{
	pthread_mutex_lock(&int_mutex);
	RZ = 0;
	RP = 0;
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
void int_clear(int x)
{
	uint32_t mask;
	LOG(L_INT, 20, "Clear: %lld (%s)", x, log_int_name[x]);
	mask = ~((1 << (31 - x)));
	pthread_mutex_lock(&int_mutex);
	RZ &= mask;
	RP &= mask;
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
void int_put_nchan(uint16_t r)
{
	LOG(L_INT, 20, "Set non-channel to: %d", r);
	pthread_mutex_lock(&int_mutex);
	RZ = (RZ & 0b00000000000011111111111111110000) | ((r & 0b1111111111110000) << 16) | (r & 0b0000000000001111);
	pthread_mutex_unlock(&int_mutex);
	int_update_rp();
}

// -----------------------------------------------------------------------
uint16_t int_get_nchan()
{
	uint32_t r;
	pthread_mutex_lock(&int_mutex);
	r = RZ;
	pthread_mutex_unlock(&int_mutex);
	return ((r & 0b11111111111100000000000000000000) >> 16) | (r & 0b00000000000000000000000000001111);
}

// -----------------------------------------------------------------------
void int_serve()
{
	uint32_t rp;
	int probe = 31;
	int interrupt;
	uint16_t int_addr;
	uint16_t int_spec = 0;
	uint16_t sp;

	pthread_mutex_lock(&int_mutex);
	rp = RP;
	pthread_mutex_unlock(&int_mutex);

	// find highest interrupt to serve
	while ((probe > 0) && !(rp & (1 << probe))) {
		probe--;
	}

	// this is the interrupt we're going to serve
	interrupt = 31 - probe;
	if (!mem_cpu_get(0, 64+interrupt, &int_addr)) return;

	// get interrupt specification if it's from channel
	if ((interrupt >= 12) && (interrupt <= 27)) {
		io_chan[interrupt-12]->cmd(io_chan[interrupt-12], IO_IN, 1<<11, &int_spec);
	}

	LOG(L_INT, 1, "Serve: %d (%s, spec: %i) -> 0x%04x / return: 0x%04x", interrupt, log_int_name[interrupt], int_spec, int_addr, regs[R_IC]);

	// put system status on stack
	if (!mem_cpu_get(0, 97, &sp)) return;
	if (!mem_cpu_put(0, sp, regs[R_IC])) return;
	if (!mem_cpu_put(0, sp+1, regs[0])) return;
	if (!mem_cpu_put(0, sp+2, regs[R_SR])) return;
	if (!mem_cpu_put(0, sp+3, int_spec)) return;
	if (!mem_cpu_put(0, 97, sp+4)) return;

	regs[0] = 0;
	int_clear(interrupt);
	regs[R_IC] = int_addr;
	regs[R_SR] &= int_int2mask[interrupt]; // put mask and clear Q
	if (cpu_mod) regs[R_SR] &= MASK_EX; // put extended mask if cpu_mod
}


// vim: tabstop=4 shiftwidth=4 autoindent
