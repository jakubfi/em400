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
#include <semaphore.h>

#include "cpu/cpu.h"
#include "cpu/registers.h"
#include "mem/mem.h"
#include "cpu/interrupts.h"
#include "io/io.h"
#include "io/chan.h"

#ifdef WITH_DEBUGGER
#include "debugger/debugger.h"
#endif
#include "debugger/log.h"

uint32_t RZ;
uint32_t RP;
uint32_t int_mask;

int int_timer = INT_TIMER;
int int_extra = INT_EXTRA;

sem_t int_ready;
pthread_mutex_t int_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t int_cond = PTHREAD_COND_INITIALIZER;

// for extending RM into 32-bit xmask
static const uint32_t int_rm2xmask[10] = {
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
static const int int_int2mask[32] = {
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
	int semval;
	RP = RZ & int_mask;
	if (RP) {
		sem_getvalue(&int_ready, &semval);
		while (semval < 1) {
			sem_post(&int_ready);
			sem_getvalue(&int_ready, &semval);
		}
		pthread_cond_signal(&int_cond);
	} else {
		while (!sem_trywait(&int_ready));
	}
}

// -----------------------------------------------------------------------
void int_update_mask()
{
	int i;
	uint32_t xmask = 0b10000000000000000000000000000000;

	pthread_mutex_lock(&int_mutex);
	for (i=0 ; i<10 ; i++) {
		if (regs[R_SR] & (1 << (15-i))) {
			xmask |= int_rm2xmask[i];
		}
	}
	int_mask = xmask;
	int_update_rp();
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
	RZ |= INT_BIT(x);
	int_update_rp();
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
void int_clear_all()
{
	pthread_mutex_lock(&int_mutex);
	RZ = 0;
	int_update_rp();
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
void int_clear(int x)
{
	LOG(L_INT, 20, "Clear: %lld (%s)", x, log_int_name[x]);
	pthread_mutex_lock(&int_mutex);
	RZ &= ~INT_BIT(x);
	int_update_rp();
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
void int_put_nchan(uint16_t r)
{
	LOG(L_INT, 20, "Set non-channel to: %d", r);
	pthread_mutex_lock(&int_mutex);
	RZ = (RZ & 0b00000000000011111111111111110000) | ((r & 0b1111111111110000) << 16) | (r & 0b0000000000001111);
	int_update_rp();
	pthread_mutex_unlock(&int_mutex);
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
	int probe = 31;
	int interrupt;
	uint16_t int_addr;
	uint16_t int_spec = 0;
	uint16_t sr_mask;

	pthread_mutex_lock(&int_mutex);
	// find highest interrupt to serve
	while ((probe > 0) && !(RP & (1 << probe))) {
		probe--;
	}
	interrupt = 31 - probe;
	// clear interrupt, we update rp together with mask later
	RZ &= ~INT_BIT(interrupt);
	pthread_mutex_unlock(&int_mutex);

	if (!mem_cpu_get(0, 64+interrupt, &int_addr)) return;

	// get interrupt specification if it's from channel
	if ((interrupt >= 12) && (interrupt <= 27)) {
		io_chan[interrupt-12]->cmd(io_chan[interrupt-12], IO_IN, 1<<11, &int_spec);
	}

	LOG(L_INT, 1, "Serve: %d (%s, spec: %i) -> 0x%04x / return: 0x%04x", interrupt, log_int_name[interrupt], int_spec, int_addr, regs[R_IC]);

	// put system status on stack
	sr_mask = int_int2mask[interrupt] & MASK_Q; // put mask and clear Q
	if (cpu_mod && (interrupt >= 12) && (interrupt <= 27)) sr_mask &= MASK_EX; // put extended mask if cpu_mod
	if (!cpu_ctx_switch(int_spec, int_addr, sr_mask)) return;
#ifdef WITH_DEBUGGER
	log_int_level -= 4;
#endif
}


// vim: tabstop=4 shiftwidth=4 autoindent
