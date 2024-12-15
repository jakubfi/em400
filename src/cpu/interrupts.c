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
#include <stdatomic.h>

#include "cpu/cpu.h"
#include "mem/mem.h"
#include "cpu/interrupts.h"
#include "io/io.h"

#include "log.h"

#include "ectl.h" // for global constants

uint32_t rz;
uint32_t rp;
uint32_t int_mask;

pthread_mutex_t int_mutex = PTHREAD_MUTEX_INITIALIZER;

#define INT_BIT(x) (1UL << (31 - x))

#define RZ_CHAN_BITMASK			0b00000000000011111111111111110000
#define RZ_NCHAN_HIGH_BITMASK	0b11111111111100000000000000000000
#define RZ_NCHAN_LOW_BITMASK	0b00000000000000000000000000001111

#define R_NCHAN_HIGH_BITMASK	0b1111111111110000
#define R_NCHAN_LOW_BITMASK		0b0000000000001111

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

// bit masks (to use on RM) for each interrupt
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

static const char *int_names[] = {
	"CPU power loss",
	"memory parity error",
	"no memory",
	"2nd CPU high",
	"ext power loss",
	"clock (special)",
	"illegal instruction",
	"AWP div overflow",
	"AWP underflow",
	"AWP overflow",
	"AWP div/0",
	"special (clock)",
	"chan 0",
	"chan 1",
	"chan 2",
	"chan 3",
	"chan 4",
	"chan 5",
	"chan 6",
	"chan 7",
	"chan 8",
	"chan 9",
	"chan 10",
	"chan 11",
	"chan 12",
	"chan 13",
	"chan 14",
	"chan 15",
	"OPRQ",
	"2nd CPU low",
	"software high",
	"software low"
};

// -----------------------------------------------------------------------
static void int_update_rp()
{
	// function called under mutex
	rp = rz & int_mask;
	cpu_wake_up();
}

// -----------------------------------------------------------------------
void int_update_mask(uint16_t mask)
{
	int i;
	uint32_t xmask = 1 << 31;

	for (i=0 ; i<10 ; i++) {
		if (mask & (1 << (9 - i))) {
			xmask |= int_rm2xmask[i];
		}
	}

	pthread_mutex_lock(&int_mutex);
	int_mask = xmask;
	int_update_rp();
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
void int_set(int x)
{
	LOG(L_INT, "Set interrupt: %i (%s)", x, int_names[x]);

	pthread_mutex_lock(&int_mutex);
	rz |= INT_BIT(x);
	int_update_rp();
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
void int_clear_all()
{
	pthread_mutex_lock(&int_mutex);
	rz = 0;
	int_update_rp();
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
void int_clear(int x)
{
	LOG(L_INT, "Clear interrupt: %i (%s)", x, int_names[x]);

	pthread_mutex_lock(&int_mutex);
	rz &= ~INT_BIT(x);
	int_update_rp();
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
void int_put_nchan(uint16_t r)
{
	LOG(L_INT, "Set non-channel interrupts to: %d", r);

	pthread_mutex_lock(&int_mutex);
	rz = (rz & RZ_CHAN_BITMASK) | ((r & R_NCHAN_HIGH_BITMASK) << 16) | (r & R_NCHAN_LOW_BITMASK);
	int_update_rp();
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
uint16_t int_get_nchan()
{
	uint32_t rz_tmp;
	pthread_mutex_lock(&int_mutex);
	rz_tmp = rz;
	pthread_mutex_unlock(&int_mutex);
	return ((rz_tmp & RZ_NCHAN_HIGH_BITMASK) >> 16) | (rz_tmp & RZ_NCHAN_LOW_BITMASK);
}

// -----------------------------------------------------------------------
uint16_t int_get_chan()
{
	uint32_t rz_tmp;
	pthread_mutex_lock(&int_mutex);
	rz_tmp = rz;
	pthread_mutex_unlock(&int_mutex);
	return rz_tmp >> 4;
}

// -----------------------------------------------------------------------
void int_serve()
{
	// find highest interrupt to serve
	unsigned interrupt = 31;
	unsigned i = rp;
	while (i >>= 1) interrupt--;

	// clear interrupt; rp gets updated int context switch, together with interrupt mask
	pthread_mutex_lock(&int_mutex);
	rz &= ~INT_BIT(interrupt);
	pthread_mutex_unlock(&int_mutex);

	// get interrupt vector
	uint16_t int_vec;
	if (!cpu_mem_read_1(false, INT_VECTORS + interrupt, &int_vec)) return;

	LOG(L_INT, "Serve interrupt: %i (%s) -> 0x%04x", interrupt, int_names[interrupt], int_vec);

	// get new interrupt mask for the given interrupt
	uint16_t int_mask = int_int2mask[interrupt];

	// get interrupt specification for channel interrupts
	uint16_t int_spec = 0;
	if ((interrupt >= 12) && (interrupt < 12 + 16)) {
		io_get_intspec(interrupt - 12, &ac);
		int_spec = ac;
		// extend interrupt mask if cpu_mod is enabled
		if (cpu_mod_active) int_mask &= MASK_EX;
	}

	// switch context
	cpu_ctx_switch(int_spec, int_vec, int_mask);

	if (LOG_ENABLED) log_intlevel_inc();
}

// vim: tabstop=4 shiftwidth=4 autoindent
