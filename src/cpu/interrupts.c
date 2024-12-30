//  Copyright (c) 2012-2024 Jakub Filipowicz <jakubf@gmail.com>
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
#include <stdbool.h>

#include "cpu/cpu.h"
#include "mem/mem.h"
#include "cpu/interrupts.h"
#include "io/io.h"

#include "log.h"


uint32_t rz;
// IRQ is atomic because cpu thread needs quick, frequent access
// order relaxed, cpu sychronizes with IO on rz
_Atomic bool irq;
static uint32_t int_xmask;

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
	"external power loss",
	"clock (or special)",
	"illegal instruction",
	"AWP div overflow",
	"AWP underflow",
	"AWP overflow",
	"AWP div/0",
	"special (or clock)",
	"CH1", "CH0", "CH2", "CH3", "CH4", "CH5", "CH6", "CH7",
	"CH8", "CH9", "CH10", "CH11", "CH12", "CH13", "CH14", "CH15",
	"OPRQ",
	"2nd CPU low",
	"software high",
	"software low"
};


// -----------------------------------------------------------------------
void int_update_xmask()
{
	uint32_t tmp_xmask = 1 << 31; // non-maskable interrupt is always enabled

	for (int i=0 ; i<10 ; i++) {
		if (rm & (1 << (9 - i))) {
			tmp_xmask |= int_rm2xmask[i];
		}
	}

	pthread_mutex_lock(&int_mutex);
	int_xmask = tmp_xmask;
	atomic_store_explicit(&irq, rz & int_xmask, memory_order_relaxed);
	cpu_wake_up();
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
void int_set(int int_num)
{
	LOG(L_INT, "Set interrupt: %i (%s)", int_num, int_names[int_num]);

	pthread_mutex_lock(&int_mutex);
	rz |= INT_BIT(int_num);
	atomic_store_explicit(&irq, rz & int_xmask, memory_order_relaxed);
	cpu_wake_up();
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
void int_clear_all()
{
	pthread_mutex_lock(&int_mutex);
	rz = 0;
	atomic_store_explicit(&irq, rz & int_xmask, memory_order_relaxed);
	cpu_wake_up();
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
void int_clear(int int_num)
{
	LOG(L_INT, "Clear interrupt: %i (%s)", int_num, int_names[int_num]);

	pthread_mutex_lock(&int_mutex);
	rz &= ~INT_BIT(int_num);
	atomic_store_explicit(&irq, rz & int_xmask, memory_order_relaxed);
	cpu_wake_up();
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
void int_put_nchan(uint16_t r)
{
	LOG(L_INT, "Set non-channel interrupts to: %d", r);

	pthread_mutex_lock(&int_mutex);
	rz = (rz & RZ_CHAN_BITMASK) | ((r & R_NCHAN_HIGH_BITMASK) << 16) | (r & R_NCHAN_LOW_BITMASK);
	atomic_store_explicit(&irq, rz & int_xmask, memory_order_relaxed);
	cpu_wake_up();
	pthread_mutex_unlock(&int_mutex);
}

// -----------------------------------------------------------------------
uint16_t int_get_nchan()
{
	pthread_mutex_lock(&int_mutex);
	uint32_t rz_tmp = rz;
	pthread_mutex_unlock(&int_mutex);

	return ((rz_tmp & RZ_NCHAN_HIGH_BITMASK) >> 16) | (rz_tmp & RZ_NCHAN_LOW_BITMASK);
}

// -----------------------------------------------------------------------
uint16_t int_get_chan()
{
	pthread_mutex_lock(&int_mutex);
	uint32_t rz_tmp = rz;
	pthread_mutex_unlock(&int_mutex);

	return (rz_tmp >> 4) & 0xffff;
}

// -----------------------------------------------------------------------
void int_serve()
{
	pthread_mutex_lock(&int_mutex);
	// find highest interrupt to serve
	unsigned interrupt = 31;
	uint32_t rp = rz & int_xmask;
	while (rp >>= 1) interrupt--;
	// clear interrupt; irq gets updated int context switch, together with interrupt mask
	rz &= ~INT_BIT(interrupt);
	pthread_mutex_unlock(&int_mutex);

	// get interrupt vector
	uint16_t int_vec;
	if (!cpu_mem_read_1(false, INT_VECTORS + interrupt, &int_vec)) return;

	LOG(L_INT, "Serve interrupt: %i (%s) -> 0x%04x", interrupt, int_names[interrupt], int_vec);

	// get new interrupt mask for the given interrupt
	uint16_t new_rm = int_int2mask[interrupt];

	// get interrupt specification for channel interrupts
	uint16_t int_spec = 0;
	if ((interrupt >= 12) && (interrupt < 12 + 16)) {
		io_get_intspec(interrupt - 12, &ac);
		int_spec = ac;
		// extend interrupt mask if cpu_mod is enabled
		if (cpu_mod_active) new_rm &= MASK_EX;
	}

	// switch context
	int_ctx_switch(int_spec, int_vec, new_rm);

	if (LOG_ENABLED) log_intlevel_inc();
}

// -----------------------------------------------------------------------
void int_ctx_switch(uint16_t int_spec, uint16_t new_ic, uint16_t new_rm)
{
	if (!cpu_mem_read_1(false, STACK_POINTER, &ar)) return;

	LOG(L_CPU,
		"Store current process context [IC: 0x%04x, R0: 0x%04x, SR: 0x%04x, 0x%04x] @ 0x%04x, set new IC: 0x%04x",
		ic, r[0], SR_READ(), int_spec, ar, new_ic
	);

	uint16_t vector[] = { ic, r[0], SR_READ(), int_spec };
	for (int i=0 ; i<4 ; i++, ar++) {
		if (!cpu_mem_write_1(false, ar, vector[i])) return;
	}
	if (!cpu_mem_write_1(false, STACK_POINTER, ar)) return;

	r[0] = 0;
	ic = new_ic;
	q = false;
	rm &= new_rm;
	int_update_xmask();
}

// -----------------------------------------------------------------------
void int_ctx_restore(bool barnb)
{
	uint16_t sr_tmp;
	uint16_t *vector[] = { &ic, r+0, &sr_tmp };
	for (int i=0 ; i<3 ; i++, ar++) {
		if (!cpu_mem_read_1(barnb, ar, vector[i])) return;
	}
	SR_WRITE(sr_tmp);
	int_update_xmask();
	LOG(L_CPU,
		"Loaded process context from @ 0x%04x: [IC: 0x%04x, R0: 0x%04x, SR: 0x%04x]",
		ar-3, vector[0], vector[1], vector[2]
	);
}

// vim: tabstop=4 shiftwidth=4 autoindent
