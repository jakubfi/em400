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
#include "mem/mem.h"
#include "cpu/interrupts.h"
#include "io/io.h"

#include "log.h"
#include "atomic.h"

#include "ectl.h" // for global constants

uint32_t RZ;
uint32_t RP;
uint32_t int_mask;

pthread_mutex_t int_mutex = PTHREAD_MUTEX_INITIALIZER;

#define INT_BIT(x) (1UL << (31 - x))

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
	// under mutex
	RP = RZ & int_mask;
	if (RP) {
		cpu_state_change(ECTL_STATE_RUN, ECTL_STATE_WAIT);
	}
}

// -----------------------------------------------------------------------
void int_update_mask(uint16_t mask)
{
	int i;
	uint32_t xmask = 0b10000000000000000000000000000000;

	for (i=0 ; i<10 ; i++) {
		if (mask & (1 << (9-i))) {
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
	LOG(L_INT, "Clear interrupt: %i (%s)", x, int_names[x]);

	pthread_mutex_lock(&int_mutex);
	RZ &= ~INT_BIT(x);
	int_update_rp();
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
void int_put_nchan(uint16_t r)
{
	LOG(L_INT, "Set non-channel interrupts to: %d", r);

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
uint16_t int_get_chan()
{
	uint32_t r;
	pthread_mutex_lock(&int_mutex);
	r = RZ;
	pthread_mutex_unlock(&int_mutex);
	return (r >> 4) & 0xffff;
}

// -----------------------------------------------------------------------
void int_serve()
{
	int probe = 31;
	int interrupt;
	uint16_t int_vec;
	uint16_t int_spec = 0;
	uint16_t int_mask;

	// find highest interrupt to serve
	while ((probe > 0) && !(RP & (1 << probe))) {
		probe--;
	}
	interrupt = 31 - probe;

	// clear interrupt, we update rp together with mask upon ctx switch
	pthread_mutex_lock(&int_mutex);
	RZ &= ~INT_BIT(interrupt);
	pthread_mutex_unlock(&int_mutex);

	// get interrupt vector
	if (!cpu_mem_get(0, 64+interrupt, &int_vec)) return;

	LOG(L_INT, "Serve interrupt: %i (%s) -> 0x%04x", interrupt, int_names[interrupt], int_vec);

	// get interrupt specification for channel interrupts
	if ((interrupt >= 12) && (interrupt <= 27)) {
		io_get_intspec(interrupt-12, &int_spec);
	}

	int_mask = int_int2mask[interrupt]; // get new interrupt mask for the given interrupt
	if (cpu_mod_active && (interrupt >= 12) && (interrupt <= 27)) {
		// extend interrupt mask if cpu_mod is enabled
		int_mask &= MASK_EX;
	}

	// switch context
	if (!cpu_ctx_switch(int_spec, int_vec, int_mask)) return;

	if (LOG_ENABLED) {
		log_intlevel_inc();
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
