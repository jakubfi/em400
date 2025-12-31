//  Copyright (c) 2013-2025 Jakub Filipowicz <jakubf@gmail.com>
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
#include <stdbool.h>
#include <strings.h>
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "io/defs.h"
#include "io/cchar/cchar.h"
#include "io/cchar/uzfx.h"
#include "log.h"

#include "io/dev2/sp45de.h"

enum uzfx_states {
	UZFX_ST0_IDLE,				// St.0 idle state
	UZFX_ST1_SECT_RD,			// St.1 disk to buffer read
	UZFX_ST2_BUF_RD,			// St.2 buffer to cpu read
	UZFX_ST2_BUF_RD_CANCEL,		// St.2 cancel current buffer read
	UZFX_ST3_BUF_WR_START,		// St.3 start buffer write
	UZFX_ST3_BUF_WR_CANCEL,		// St.3 start buffer cancel
	UZFX_ST5_BUF_WR,			// cpu to buffer write St.3 (bad sector mark, F8) St.5 (regular data FB)
	UZFX_ST7_SECT_WR,			// buffer to disk write St.6 (with check), St.7 (no check)
	UZFX_STX_QUIT,
};

static const char *uzfx_state_names[] = {
	[UZFX_ST0_IDLE] = "idle",
	[UZFX_ST1_SECT_RD] = "sector read",
	[UZFX_ST2_BUF_RD] = "buffer read",
	[UZFX_ST2_BUF_RD_CANCEL] = "buffer read cancel",
	[UZFX_ST3_BUF_WR_START] = "buffer write start",
	[UZFX_ST3_BUF_WR_CANCEL] = "buffer write cancel",
	[UZFX_ST5_BUF_WR] = "buffer write",
	[UZFX_ST7_SECT_WR] = "sector write",
	[UZFX_STX_QUIT] = "quit",
};


enum uzfx_ou_commands {
	UZCF_CMD_RESET	= 0b100000, // reset
	UZFX_CMD_DETACH	= 0b101000, // detach (soft reset)
	UZFX_CMD_WRITE	= 0b110000, // write
	UZFX_CMD_CTL_W	= 0b111000, // control for write
	UZFX_CMD_CTL_WC	= 0b111100, // control for write+check
	UZFX_CMD_CTL_B	= 0b111110, // control for bad sector
	UZFX_CMD_CTL_R	= 0b111010, // control for read
};

enum uzfx_in_commands {
	UZFX_CMD_SPU	= 0b100000, // check device
	UZFX_CMD_READ	= 0b101000, // read
};
#define UZFX_CMD_QUIT -1

#define UZFX_INT_NONE 0
enum uzfx_interrupt_priorities {
	UZFX_INT_SECT_NOT_FOUND, // highest prio
	UZFX_INT_CRC_ERR,
	UZFX_INT_SECT_BAD,
	UZFX_INT_HW_ERR,
	UZFX_INT_DISK_END,
	UZFX_INT_READY, // lowest prio
};
static const int uzfx_interrupt_specs[] = {
	0b11010, // sector not found
	0b01010, // data CRC error
	0b10010, // sector marked as bad
	0b00010, // hardware error
	0b00100, // track==73 && sector==26
	0b00001, // device is ready
};

enum uzfx_drives {
	UZFX_DRV_0	= 0b000 << 13,
	UZFX_DRV_1	= 0b001 << 13,
	UZFX_DRV_2	= 0b100 << 13,
	UZFX_DRV_3	= 0b101 << 13,
};

enum uzfx_sides {
	UZFX_SIDE_A	= 0 << 12,
	UZFX_SIDE_B	= 1 << 12,
};

#define UZFX_ADDR_TRACK(n) (n << 5)
#define UZFX_ADDR_SECTOR(n) (n)

#define INITIAL_ADDRESS (UZFX_DRV_0 | UZFX_SIDE_A | UZFX_ADDR_TRACK(1) | UZFX_ADDR_SECTOR(1))

typedef struct uzfx uzfx_t;
struct uzfx {
	cchar_unit_t base;
	int drive, side, track, sector;
	pthread_t worker;
	pthread_mutex_t state_mutex;
	pthread_cond_t state_cond;
	int state;
	int operation;
	bool pending_buf_write;
	bool interrupt_required;
	int interrupts;
	sp45de_t *sp45de;
};

static void * uzfx_worker_loop(void *ptr);
static void uzfx_reset_state(uzfx_t *u);
void uzfx_shutdown(cchar_unit_t *unit);
void uzfx_reset(cchar_unit_t *unit);
int uzfx_cmd(cchar_unit_t *unit, int dir, int cmd, uint16_t *r_arg);
int uzfx_intspec(cchar_unit_t *unit);
bool uzfx_has_interrupt(cchar_unit_t *unit);

// -----------------------------------------------------------------------
static void uzfx_set_address(uzfx_t *uzfx, uint16_t addr)
{
	int drive = (addr >> 13) & 0b111;
	uzfx->drive = (drive & 1) | ((drive >> 1) & 2);
	uzfx->side = (addr >> 12) & 1;
	uzfx->track = (addr >> 5) & 0b1111111;
	uzfx->sector = addr & 0b11111;

	LOG(L_UZFX, "Setting new address: drive: %i, side: %i, track: %i, sector: %i", uzfx->drive, uzfx->side, uzfx->track, uzfx->sector);
}

// -----------------------------------------------------------------------
cchar_unit_t * uzfx_create(int dev_num, em400_dev_t *dev)
{
	if (dev->type != EM400_DEV_SP45DE) {
		LOGERR("UZFX can only connect SP45DE");
		return NULL;
	}

	uzfx_t *uzfx = (uzfx_t *) calloc(1, sizeof(uzfx_t));
	if (!uzfx) {
		LOGERR("UZFX unit %i failed to allocate memory for its structure", dev_num);
		goto fail;
	}

	uzfx->sp45de = (sp45de_t *) dev;

	uzfx->base.num = dev_num;
	uzfx->base.shutdown = uzfx_shutdown;
	uzfx->base.reset = uzfx_reset;
	uzfx->base.cmd = uzfx_cmd;
	uzfx->base.intspec = uzfx_intspec;
	uzfx->base.has_interrupt = uzfx_has_interrupt;

	if (pthread_mutex_init(&uzfx->state_mutex, NULL)) {
		LOGERR("UZFX failed to initialize status mutex.");
		goto fail;
	}

	if (pthread_cond_init(&uzfx->state_cond, NULL)) {
		LOGERR("UZFX failed to initialize status conditional");
		goto fail;
	}

	if (pthread_create(&uzfx->worker, NULL, uzfx_worker_loop, uzfx)) {
		LOGERR("UZFX failed to spawn worker thread.");
		goto fail;
	}

	uzfx_reset_state(uzfx);

	return (cchar_unit_t *) uzfx;

fail:
	uzfx_shutdown((cchar_unit_t *) uzfx);
	return NULL;
}

// -----------------------------------------------------------------------
static void uzfx_reset_state(uzfx_t *uzfx)
{
	pthread_mutex_lock(&uzfx->state_mutex);
	uzfx_set_address(uzfx, INITIAL_ADDRESS);
	uzfx->state = UZFX_ST0_IDLE;
	uzfx->pending_buf_write = false;
	uzfx->interrupt_required = false;
	uzfx->interrupts = UZFX_INT_NONE;
	pthread_mutex_unlock(&uzfx->state_mutex);
}

// -----------------------------------------------------------------------
void uzfx_shutdown(cchar_unit_t *unit)
{
	uzfx_t *uzfx = (uzfx_t *) unit;
	if (!uzfx) return;

	pthread_mutex_lock(&uzfx->state_mutex);
	uzfx->state = UZFX_STX_QUIT;
	pthread_cond_signal(&uzfx->state_cond);
	pthread_mutex_unlock(&uzfx->state_mutex);

	if (uzfx->worker) pthread_join(uzfx->worker, NULL);
	pthread_mutex_destroy(&uzfx->state_mutex);
	pthread_cond_destroy(&uzfx->state_cond);

	free(uzfx);
}

// -----------------------------------------------------------------------
void uzfx_reset(cchar_unit_t *unit)
{
	uzfx_t *uzfx = (uzfx_t *) unit;
	LOG(L_UZFX, "Reset");
	uzfx_reset_state(uzfx);
	cchar_int_cancel(uzfx->base.chan, uzfx->base.num);
}

// -----------------------------------------------------------------------
static bool uzfx_address_advance(uzfx_t *uzfx)
{
	// TODO: where to advance? where to check for last track? where to set interrupt?
	uzfx->sector++;

	if (uzfx->sector > SP45DE_SECTOR_PER_TRACK) {
		uzfx->sector = 1;
		uzfx->track++;
		if (uzfx->track > SP45DE_TRACK_LAST) {
			uzfx->track = 1;
			LOG(L_UZFX, "Disk end");
			return true;
		}
	}

	LOG(L_UZFX, "Advanced to track: %i, sector: %i", uzfx->track, uzfx->sector);

	return false;
}

// -----------------------------------------------------------------------
static void * uzfx_worker_loop(void *ptr)
{
	uzfx_t *uzfx = (uzfx_t *) ptr;
	bool quit = false;

	while (!quit) {
		pthread_mutex_lock(&uzfx->state_mutex);
		// last two states are not handled here
		// TODO: multithreading fixes... (or just migrate to libuv)
		while ((uzfx->state == UZFX_ST0_IDLE) || (uzfx->state == UZFX_ST2_BUF_RD) || (uzfx->state == UZFX_ST5_BUF_WR)) {
			LOG(L_UZFX, "Worker waiting for state change");
			pthread_cond_wait(&uzfx->state_cond, &uzfx->state_mutex);
		}
		int interrupt = UZFX_INT_NONE;
		int state = uzfx->state;

		LOG(L_UZFX, "Worker processing state: %s", uzfx_state_names[state]);
		switch (state) {
			case UZFX_STX_QUIT:

				quit = true;
				break;
			case UZFX_ST1_SECT_RD:
				if ((uzfx->sector == 26) && (uzfx->track == 73)) {
					interrupt |= 1 << UZFX_INT_DISK_END;
				}
				if (sp45de_blk_read(uzfx->sp45de, uzfx->drive, uzfx->track, uzfx->sector) != E_OK) {
					interrupt |= 1 << UZFX_INT_HW_ERR;
					uzfx->state = UZFX_ST0_IDLE;
				} else {
					interrupt |= 1 << UZFX_INT_READY;
					uzfx->state = UZFX_ST2_BUF_RD;
				}
				break;
			case UZFX_ST2_BUF_RD_CANCEL:
				uint8_t c;
				while (sp45de_read(uzfx->sp45de, &c) == SP45DE_BUF_OK);
				interrupt |= 1 << UZFX_INT_READY;
				uzfx->state = UZFX_ST0_IDLE;
				uzfx_address_advance(uzfx); // TODO: error checking?
				break;
			case UZFX_ST3_BUF_WR_START:
				// TODO: start the engine
				// TODO: ustalenie sposobu/rodzaju zapisu
				if ((uzfx->sector == 26) && (uzfx->track == 73)) {
					interrupt |= 1 << UZFX_INT_DISK_END;
				}
				interrupt |= 1 << UZFX_INT_READY;
				uzfx->state = UZFX_ST5_BUF_WR;
				break;
			case UZFX_ST3_BUF_WR_CANCEL:
				while (sp45de_write(uzfx->sp45de, 0) == SP45DE_BUF_OK);
				interrupt |= 1 << UZFX_INT_READY;
				// fallthrough
			case UZFX_ST7_SECT_WR:
				sp45de_blk_write(uzfx->sp45de, uzfx->drive, uzfx->track, uzfx->sector);
				uzfx_address_advance(uzfx); // TODO: error checking?
				// TODO: jeśli z kontrolą - odczyt
				if (uzfx->interrupt_required) {
					interrupt |= 1 << UZFX_INT_READY;
				}
				// during sector write, another buffer write came. honor it.
				if (uzfx->pending_buf_write) {
					uzfx->state = UZFX_ST3_BUF_WR_START;
					uzfx->pending_buf_write = false;
				} else {
					uzfx->state = UZFX_ST0_IDLE;
				}
				break;
		}

		if (uzfx->state != state) {
			LOG(L_UZFX, "Worker changed state to: %s", uzfx_state_names[uzfx->state]);
		}

		if (interrupt != UZFX_INT_NONE) {
			uzfx->interrupts = interrupt;
			uzfx->interrupt_required = false;
		}
		pthread_mutex_unlock(&uzfx->state_mutex);

		if (interrupt != UZFX_INT_NONE) {
			LOG(L_UZFX, "Worker sending interrupt");
			cchar_int_trigger(uzfx->base.chan);
		}

	}

	LOG(L_UZFX, "Leaving worker loop");
	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
bool uzfx_has_interrupt(cchar_unit_t *unit)
{
	uzfx_t *uzfx = (uzfx_t *) unit;

	pthread_mutex_lock(&uzfx->state_mutex);
	int interrupt = uzfx->interrupts;
	pthread_mutex_unlock(&uzfx->state_mutex);

	return interrupt ? true : false;
}

// -----------------------------------------------------------------------
int uzfx_intspec(cchar_unit_t *unit)
{
	uzfx_t *uzfx = (uzfx_t *) unit;

	int spec = UZFX_INT_NONE;

	pthread_mutex_lock(&uzfx->state_mutex);
	for (int shift=0 ; shift<=5 ; shift++) {
		if (uzfx->interrupts & (1<<shift)) {
			spec = uzfx_interrupt_specs[shift];
			uzfx->interrupts &= ~(1<<shift);
			break;
		}
	}
	pthread_mutex_unlock(&uzfx->state_mutex);

	return spec;
}

// -----------------------------------------------------------------------
static int uzfx_cmd_read(cchar_unit_t *unit, uint16_t *r_arg)
{
	uzfx_t *uzfx = (uzfx_t *) unit;
	int io_ret;

	pthread_mutex_lock(&uzfx->state_mutex);
	int old_state = uzfx->state;
	LOG(L_UZFX, "command: buf read 0x%02x (state: %s)", *r_arg, uzfx_state_names[uzfx->state]);
	switch (uzfx->state) {
		case UZFX_ST0_IDLE:
			uzfx->state = UZFX_ST1_SECT_RD;
			pthread_cond_signal(&uzfx->state_cond);
			io_ret = IO_EN;
			break;
		case UZFX_ST2_BUF_RD:
			uint8_t c;
			if (sp45de_read(uzfx->sp45de, &c) == SP45DE_BUF_END) {
				// last byte, set next sector address
				uzfx_address_advance(uzfx);
				uzfx->state = UZFX_ST0_IDLE;
			}
			*r_arg = c;
			uzfx->interrupts = UZFX_INT_NONE;
			io_ret = IO_OK;
			break;
		default:
			io_ret = IO_EN;
			break;
	}
	if (uzfx->state != old_state) {
		LOG(L_UZFX, "State changed to: %s", uzfx_state_names[uzfx->state]);
	}
	pthread_mutex_unlock(&uzfx->state_mutex);

	return io_ret;
}

// -----------------------------------------------------------------------
static int uzfx_cmd_write(cchar_unit_t *unit, const uint16_t *r_arg)
{
	uzfx_t *uzfx = (uzfx_t *) unit;
	int io_ret;

	pthread_mutex_lock(&uzfx->state_mutex);
	int old_state = uzfx->state;
	LOG(L_UZFX, "command: buf write 0x%02x (state: %s)", (uint8_t) *r_arg, uzfx_state_names[uzfx->state]);
	switch (uzfx->state) {
		case UZFX_ST0_IDLE:
			uzfx->state = UZFX_ST3_BUF_WR_START;
			pthread_cond_signal(&uzfx->state_cond);
			io_ret = IO_EN;
			break;
		case UZFX_ST5_BUF_WR:
			if (sp45de_write(uzfx->sp45de, (uint8_t) *r_arg) == SP45DE_BUF_END) {
				uzfx->state = UZFX_ST7_SECT_WR;
				pthread_cond_signal(&uzfx->state_cond);
			}
			io_ret = IO_OK;
			break;
		case UZFX_ST7_SECT_WR:
			uzfx->pending_buf_write = true;
			io_ret = IO_EN;
			break;
		default:
			io_ret = IO_EN;
			break;
	}
	if (uzfx->state != old_state) {
		LOG(L_UZFX, "State changed to: %s", uzfx_state_names[uzfx->state]);
	}
	pthread_mutex_unlock(&uzfx->state_mutex);

	return io_ret;
}

// -----------------------------------------------------------------------
static int uzfx_cmd_control(cchar_unit_t *unit, const uint16_t *r_arg, int cmd)
{
	uzfx_t *uzfx = (uzfx_t *) unit;
	int io_ret;

	pthread_mutex_lock(&uzfx->state_mutex);
	LOG(L_UZFX, "command: control (state: %s)", uzfx_state_names[uzfx->state]);
	switch (uzfx->state) {
		case UZFX_ST0_IDLE:
			uzfx->operation = cmd;
			uzfx_set_address(uzfx, *r_arg);
			LOG(L_UZFX, "Set new address: drive %i, side: %i, track %i, sector %i", uzfx->drive, uzfx->side, uzfx->track, uzfx->sector);
			io_ret = IO_OK;
			break;
		default:
			// this is illegal and puts the controllen in a state that won't send an interrupt
			io_ret = IO_EN;
			break;
	}
	pthread_mutex_unlock(&uzfx->state_mutex);

	return io_ret;
}

// -----------------------------------------------------------------------
static int uzfx_cmd_reset(cchar_unit_t *unit)
{
	LOG(L_UZFX, "command: reset");
	uzfx_t *uzfx = (uzfx_t*) unit;
	uzfx_reset_state(uzfx);
	cchar_int_cancel(uzfx->base.chan, uzfx->base.num);

	return IO_OK;
}

// -----------------------------------------------------------------------
static int uzfx_cmd_spu()
{
	LOG(L_UZFX, "command: SPU");

	return IO_OK;
}

// -----------------------------------------------------------------------
static int uzfx_cmd_detach(cchar_unit_t *unit)
{
	uzfx_t *uzfx = (uzfx_t *) unit;
	int io_ret;

	// TODO: detach zakończony EN nie jest wykonanym detachem w rozumieniu następnych komend
	// (i.e. następujące "STERUJ" zakończy się EN i nie będzie się dało z tego wyjść)
	// TODO: test
	pthread_mutex_lock(&uzfx->state_mutex);
	int old_state = uzfx->state;
	LOG(L_UZFX, "command: detach (state: %s -> %s)", uzfx_state_names[old_state], uzfx_state_names[uzfx->state]);
	switch (uzfx->state) {
		case UZFX_ST0_IDLE:
			uzfx->interrupts = UZFX_INT_NONE;
			io_ret = IO_OK;
			break;
		case UZFX_ST2_BUF_RD:
			uzfx->state = UZFX_ST2_BUF_RD_CANCEL;
			pthread_cond_signal(&uzfx->state_cond);
			io_ret = IO_EN;
			break;
		case UZFX_ST5_BUF_WR:
			uzfx->state = UZFX_ST3_BUF_WR_CANCEL;
			pthread_cond_signal(&uzfx->state_cond);
			io_ret = IO_EN;
			break;
		case UZFX_ST7_SECT_WR:
			// force a buffer write after current sector write finishes
			uzfx->pending_buf_write = false;
			// in normal flow, finished sector write does not send an interrupt,
			// but since now, after failed detach CPU waits for an interrupt,
			// make sure we send it
			uzfx->interrupt_required = true;
			io_ret = IO_EN;
			break;
		default:
			io_ret = IO_EN;
			break;
	}
	if (uzfx->state != old_state) {
		LOG(L_UZFX, "State changed to: %s", uzfx_state_names[uzfx->state]);
	}
	pthread_mutex_unlock(&uzfx->state_mutex);

	return io_ret;
}

// -----------------------------------------------------------------------
int uzfx_cmd(cchar_unit_t *unit, int dir, int cmd, uint16_t *r_arg)
{
	if (dir == IO_IN) {
		switch (cmd) {
			case UZFX_CMD_SPU:
				return uzfx_cmd_spu();
			case UZFX_CMD_READ:
				return uzfx_cmd_read(unit, r_arg);
			default:
				LOG(L_UZFX, "unknown IN command: %i", cmd);
				return IO_OK;
		}
	} else {
		switch (cmd) {
			case UZCF_CMD_RESET:
				return uzfx_cmd_reset(unit);
			case UZFX_CMD_DETACH:
				return uzfx_cmd_detach(unit);
			case UZFX_CMD_WRITE:
				return uzfx_cmd_write(unit, r_arg);
			case UZFX_CMD_CTL_R:
			case UZFX_CMD_CTL_W:
			case UZFX_CMD_CTL_WC:
			case UZFX_CMD_CTL_B:
				return uzfx_cmd_control(unit, r_arg, cmd);
			default:
				LOG(L_UZFX, "unknown OUT command: %i", cmd);
				return IO_OK;
		}
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
