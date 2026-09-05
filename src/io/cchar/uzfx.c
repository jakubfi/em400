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
#include <uv.h>

#include "io/defs.h"
#include "io/cchar/cchar.h"
#include "io/cchar/uzfx.h"
#include "log.h"

#include "io/dev/sp45de.h"

extern uv_loop_t *ioloop;

enum uzfx_states {
	UZFX_ST0_IDLE,				// St.0 idle state
	UZFX_ST1_SECT_RD,			// St.1 disk to buffer read
	UZFX_ST2_BUF_RD,			// St.2 buffer to cpu read
	UZFX_ST2_BUF_RD_CANCEL,		// St.2 cancel current buffer read
	UZFX_ST3_BUF_WR_START,		// St.3 start buffer write
	UZFX_ST3_BUF_WR_CANCEL,		// St.3 start buffer cancel
	UZFX_ST5_BUF_WR,			// cpu to buffer write St.3 (bad sector mark, F8) St.5 (regular data FB)
	UZFX_ST7_SECT_WR,			// buffer to disk write St.6 (with check), St.7 (no check)
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
};


enum uzfx_ou_commands {
	UZFX_CMD_CTL_W	= 0b111000, // control for write
	UZFX_CMD_CTL_WC	= 0b111100, // control for write+check
	UZFX_CMD_CTL_B	= 0b111110, // control for bad sector
	UZFX_CMD_CTL_R	= 0b111010, // control for read
};

#define UZFX_INT_NONE 0
enum uzfx_interrupts {
	UZFX_INT_SECT_NOT_FOUND, // highest priority
	UZFX_INT_CRC_ERR,
	UZFX_INT_SECT_BAD,
	UZFX_INT_HW_ERR,
	UZFX_INT_DISK_END,
	UZFX_INT_READY,
	UZFX_INT_MAX = UZFX_INT_READY
};
static const int uzfx_interrupt_specs[] = {
	[UZFX_INT_SECT_NOT_FOUND]	= 0b11010,
	[UZFX_INT_CRC_ERR]			= 0b01010,
	[UZFX_INT_SECT_BAD]			= 0b10010,
	[UZFX_INT_HW_ERR]			= 0b00010,
	[UZFX_INT_DISK_END]			= 0b00100,
	[UZFX_INT_READY]			= 0b00001
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
	pthread_mutex_t state_mutex;
	int state;
	int operation;
	bool pending_buf_write;
	bool pending_detach_int;
	int interrupts;
	unsigned xfer_gen; // reset generation to track which "live" a transfer belongs to
	unsigned xfer_gen_active;
	bool xfer_in_flight;
	sp45de_t *sp45de;
	uv_async_t async_work;
};

static void uzfx_reset__nolock(uzfx_t *uzfx);
void uzfx_shutdown(cchar_unit_t *unit);
void uzfx_reset(cchar_unit_t *unit);
int uzfx_cmd(cchar_unit_t *unit, int dir, int cmd, uint16_t *r_arg);
int uzfx_intspec(cchar_unit_t *unit);
bool uzfx_has_interrupt(cchar_unit_t *unit);

static void uzfx_on_async_work(uv_async_t *handle);
static void uzfx_on_sector_read(void *ctx, int result);
static void uzfx_on_sector_write(void *ctx, int result);

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
static void uzfx_on_handle_close(uv_handle_t *handle)
{
	uzfx_t *uzfx = (uzfx_t *) uv_handle_get_data(handle);

	LOG(L_UZFX, "UZFX freeing resources");
	pthread_mutex_destroy(&uzfx->state_mutex);
	free(uzfx);
}

// -----------------------------------------------------------------------
static void uzfx_ioloop_teardown(uzfx_t *uzfx)
{
	if (!uv_is_closing((uv_handle_t *) &uzfx->async_work)) {
		uv_close((uv_handle_t *) &uzfx->async_work, uzfx_on_handle_close);
	}
}

// -----------------------------------------------------------------------
static int uzfx_ioloop_setup(uzfx_t *uzfx)
{
	int res = uv_async_init(ioloop, &uzfx->async_work, uzfx_on_async_work);
	if (res) {
		return LOGERR("Device %i: UZFX async_work handler init error: %s", uzfx->base.num, uv_strerror(res));
	}
	uv_handle_set_data((uv_handle_t *) &uzfx->async_work, uzfx);

	return E_OK;
}

// -----------------------------------------------------------------------
cchar_unit_t * uzfx_create(int dev_num, em400_dev_t *dev)
{
	if (dev->type != EM400_DEV_SP45DE) {
		LOGERR("Device %i: UZFX can only connect SP45DE", dev_num);
		return NULL;
	}

	uzfx_t *uzfx = (uzfx_t *) calloc(1, sizeof(uzfx_t));
	if (!uzfx) {
		LOGERR("Device %i: UZFX failed to allocate memory for its structure", dev_num);
		return NULL;
	}

	uzfx->sp45de = (sp45de_t *) dev;

	uzfx->base.num = dev_num;
	uzfx->base.shutdown = uzfx_shutdown;
	uzfx->base.reset = uzfx_reset;
	uzfx->base.cmd = uzfx_cmd;
	uzfx->base.intspec = uzfx_intspec;
	uzfx->base.has_interrupt = uzfx_has_interrupt;

	if (pthread_mutex_init(&uzfx->state_mutex, NULL)) {
		LOGERR("Device %i: UZFX failed to initialize state mutex", dev_num);
		free(uzfx);
		return NULL;
	}

	if (uzfx_ioloop_setup(uzfx) != E_OK) {
		pthread_mutex_destroy(&uzfx->state_mutex);
		free(uzfx);
		return NULL;
	}

	uzfx_reset(&uzfx->base);

	return (cchar_unit_t *) uzfx;
}

// -----------------------------------------------------------------------
static void uzfx_reset__nolock(uzfx_t *uzfx)
{
	uzfx_set_address(uzfx, INITIAL_ADDRESS);
	uzfx->state = UZFX_ST0_IDLE;
	uzfx->pending_buf_write = false;
	uzfx->pending_detach_int = false;
	uzfx->interrupts = UZFX_INT_NONE;
	uzfx->xfer_gen++;
	sp45de_motor_stop(uzfx->sp45de);
	sp45de_reset(uzfx->sp45de);
}

// -----------------------------------------------------------------------
void uzfx_shutdown(cchar_unit_t *unit)
{
	uzfx_t *uzfx = (uzfx_t *) unit;
	if (!uzfx) return;

	LOG(L_UZFX, "UZFX shutting down");

	uzfx_ioloop_teardown(uzfx);

	uzfx->sp45de->base.shutdown((em400_dev_t *) uzfx->sp45de);
}

// -----------------------------------------------------------------------
void uzfx_reset(cchar_unit_t *unit)
{
	uzfx_t *uzfx = (uzfx_t *) unit;
	LOG(L_UZFX, "Reset");
	pthread_mutex_lock(&uzfx->state_mutex);
	uzfx_reset__nolock(uzfx);
	pthread_mutex_unlock(&uzfx->state_mutex);
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
static int uzfx_disk_end_int(uzfx_t *uzfx)
{
	if ((uzfx->sector == SP45DE_SECTOR_PER_TRACK) && (uzfx->track == SP45DE_TRACK_LAST)) {
		return 1 << UZFX_INT_DISK_END;
	}
	return UZFX_INT_NONE;
}

// -----------------------------------------------------------------------
static void uzfx_int_set(uzfx_t *uzfx, int interrupts)
{
	if (interrupts == UZFX_INT_NONE) return;

	uzfx->interrupts = interrupts;
	uzfx->pending_detach_int = false;
}

// -----------------------------------------------------------------------
static int uzfx_sector_read_done(uzfx_t *uzfx, int result)
{
	int interrupt = uzfx_disk_end_int(uzfx);

	if (result != E_OK) {
		interrupt |= 1 << UZFX_INT_HW_ERR;
		uzfx->state = UZFX_ST0_IDLE;
	} else {
		interrupt |= 1 << UZFX_INT_READY;
		uzfx->state = UZFX_ST2_BUF_RD;
	}

	return interrupt;
}

// -----------------------------------------------------------------------
static int uzfx_sector_read_start(uzfx_t *uzfx)
{
	if (sp45de_blk_read(uzfx->sp45de, uzfx->drive, uzfx->track, uzfx->sector, uzfx_on_sector_read, uzfx) == E_OK) {
		uzfx->xfer_in_flight = true;
		uzfx->xfer_gen_active = uzfx->xfer_gen;
		return UZFX_INT_NONE;
	}

	return uzfx_sector_read_done(uzfx, E_ERR);
}

// -----------------------------------------------------------------------
static void uzfx_on_sector_read(void *ctx, int result)
{
	uzfx_t *uzfx = (uzfx_t *) ctx;

	pthread_mutex_lock(&uzfx->state_mutex);
	uzfx->xfer_in_flight = false;
	if (uzfx->xfer_gen_active != uzfx->xfer_gen) {
		pthread_mutex_unlock(&uzfx->state_mutex);
		LOG(L_UZFX, "Dropping sector read result after a reset");
		// loop early-returns with xfer_in_flight, need to trigger it
		// so a potential waiting transfer is picked up
		uv_async_send(&uzfx->async_work);
		return;
	}
	int interrupt = uzfx_sector_read_done(uzfx, result);
	LOG(L_UZFX, "Sector read finished, state: %s", uzfx_state_names[uzfx->state]);
	uzfx_int_set(uzfx, interrupt);
	pthread_mutex_unlock(&uzfx->state_mutex);

	if (interrupt != UZFX_INT_NONE) {
		cchar_int_trigger(uzfx->base.chan);
	}
}

// -----------------------------------------------------------------------
static int uzfx_sector_write_done(uzfx_t *uzfx)
{
	int interrupt = UZFX_INT_NONE;

	uzfx_address_advance(uzfx); // TODO: error checking?
	// TODO: jeśli z kontrolą - odczyt
	if (uzfx->pending_detach_int) {
		interrupt |= 1 << UZFX_INT_READY;
	}
	// during sector write, another buffer write came. honor it.
	if (uzfx->pending_buf_write) {
		uzfx->state = UZFX_ST3_BUF_WR_START;
		uzfx->pending_buf_write = false;
		uv_async_send(&uzfx->async_work);
	} else {
		uzfx->state = UZFX_ST0_IDLE;
	}

	return interrupt;
}

// -----------------------------------------------------------------------
static int uzfx_sector_write_start(uzfx_t *uzfx)
{
	if (sp45de_blk_write(uzfx->sp45de, uzfx->drive, uzfx->track, uzfx->sector, uzfx_on_sector_write, uzfx) == E_OK) {
		uzfx->xfer_in_flight = true;
		uzfx->xfer_gen_active = uzfx->xfer_gen;
		return UZFX_INT_NONE;
	}

	// TODO: actual error checking?
	return uzfx_sector_write_done(uzfx);
}

// -----------------------------------------------------------------------
static void uzfx_on_sector_write(void *ctx, int result)
{
	(void) result;
	uzfx_t *uzfx = (uzfx_t *) ctx;

	pthread_mutex_lock(&uzfx->state_mutex);
	uzfx->xfer_in_flight = false;
	if (uzfx->xfer_gen_active != uzfx->xfer_gen) {
		pthread_mutex_unlock(&uzfx->state_mutex);
		LOG(L_UZFX, "Dropping sector write result after a reset");
		uv_async_send(&uzfx->async_work);
		return;
	}
	int interrupt = uzfx_sector_write_done(uzfx);
	LOG(L_UZFX, "Sector write finished, state: %s", uzfx_state_names[uzfx->state]);
	uzfx_int_set(uzfx, interrupt);
	pthread_mutex_unlock(&uzfx->state_mutex);

	if (interrupt != UZFX_INT_NONE) {
		cchar_int_trigger(uzfx->base.chan);
	}
}

// -----------------------------------------------------------------------
static void uzfx_on_async_work(uv_async_t *handle)
{
	uzfx_t *uzfx = (uzfx_t *) uv_handle_get_data((uv_handle_t *) handle);
	int interrupt = UZFX_INT_NONE;

	pthread_mutex_lock(&uzfx->state_mutex);
	// an orphaned transfer still owns the drive; its completion re-triggers the loop
	if (uzfx->xfer_in_flight) {
		pthread_mutex_unlock(&uzfx->state_mutex);
		return;
	}
	int state = uzfx->state;

	LOG(L_UZFX, "Processing state: %s", uzfx_state_names[state]);
	switch (state) {
		case UZFX_ST1_SECT_RD:
			interrupt = uzfx_sector_read_start(uzfx);
			break;
		case UZFX_ST2_BUF_RD_CANCEL:
			uint8_t c;
			while (sp45de_buf_read(uzfx->sp45de, &c) == SP45DE_BUF_OK) {
			}
			interrupt = 1 << UZFX_INT_READY;
			uzfx->state = UZFX_ST0_IDLE;
			uzfx_address_advance(uzfx); // TODO: error checking?
			break;
		case UZFX_ST3_BUF_WR_START:
			// TODO: start the engine
			// TODO: ustalenie sposobu/rodzaju zapisu
			interrupt = uzfx_disk_end_int(uzfx) | (1 << UZFX_INT_READY);
			uzfx->state = UZFX_ST5_BUF_WR;
			break;
		case UZFX_ST3_BUF_WR_CANCEL:
			while (sp45de_buf_write(uzfx->sp45de, 0) == SP45DE_BUF_OK) {
			}
			// since DETACH responded with EN, we need to
			// send the interrupt once the write is done
			uzfx->pending_detach_int = true;
			uzfx->state = UZFX_ST7_SECT_WR;
			interrupt = uzfx_sector_write_start(uzfx);
			break;
		case UZFX_ST7_SECT_WR:
			interrupt = uzfx_sector_write_start(uzfx);
			break;
		default:
			break;
	}

	if (uzfx->state != state) {
		LOG(L_UZFX, "State changed to: %s", uzfx_state_names[uzfx->state]);
	}

	uzfx_int_set(uzfx, interrupt);
	pthread_mutex_unlock(&uzfx->state_mutex);

	if (interrupt != UZFX_INT_NONE) {
		LOG(L_UZFX, "Sending interrupt");
		cchar_int_trigger(uzfx->base.chan);
	}
}

// -----------------------------------------------------------------------
static bool uzfx_int_is_error(int interrupt)
{
	switch (interrupt) {
		case UZFX_INT_SECT_NOT_FOUND:
		case UZFX_INT_CRC_ERR:
		case UZFX_INT_SECT_BAD:
		case UZFX_INT_HW_ERR:
			return true;
		default:
			return false;
	}
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
	for (int interrupt=0 ; interrupt<=UZFX_INT_MAX ; interrupt++) {
		if (uzfx->interrupts & (1<<interrupt)) {
			spec = uzfx_interrupt_specs[interrupt];
			uzfx->interrupts &= ~(1<<interrupt);
			// hard errors reset the controller
			if (uzfx_int_is_error(interrupt)) {
				uzfx_reset__nolock(uzfx);
			}
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
	bool loop_trigger = false;

	pthread_mutex_lock(&uzfx->state_mutex);
	int old_state = uzfx->state;
	LOG(L_UZFX, "command: buf read 0x%02x (state: %s)", *r_arg, uzfx_state_names[uzfx->state]);
	switch (uzfx->state) {
		case UZFX_ST0_IDLE:
			uzfx->state = UZFX_ST1_SECT_RD;
			sp45de_motor_start(uzfx->sp45de);
			loop_trigger = true;
			io_ret = IO_EN;
			break;
		case UZFX_ST2_BUF_RD:
			uint8_t c;
			if (sp45de_buf_read(uzfx->sp45de, &c) == SP45DE_BUF_END) {
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

	if (loop_trigger) {
		uv_async_send(&uzfx->async_work);
	}

	return io_ret;
}

// -----------------------------------------------------------------------
static int uzfx_cmd_write(cchar_unit_t *unit, const uint16_t *r_arg)
{
	uzfx_t *uzfx = (uzfx_t *) unit;
	int io_ret;
	bool loop_trigger = false;

	pthread_mutex_lock(&uzfx->state_mutex);
	int old_state = uzfx->state;
	LOG(L_UZFX, "command: buf write 0x%02x (state: %s)", (uint8_t) *r_arg, uzfx_state_names[uzfx->state]);
	switch (uzfx->state) {
		case UZFX_ST0_IDLE:
			uzfx->state = UZFX_ST3_BUF_WR_START;
			sp45de_motor_start(uzfx->sp45de);
			loop_trigger = true;
			io_ret = IO_EN;
			break;
		case UZFX_ST5_BUF_WR:
			if (sp45de_buf_write(uzfx->sp45de, (uint8_t) *r_arg) == SP45DE_BUF_END) {
				uzfx->state = UZFX_ST7_SECT_WR;
				loop_trigger = true;
			}
			io_ret = IO_OK;
			break;
		case UZFX_ST7_SECT_WR:
			// force a buffer write after current sector write finishes
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

	if (loop_trigger) {
		uv_async_send(&uzfx->async_work);
	}

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
			sp45de_motor_start(uzfx->sp45de);
			io_ret = IO_OK;
			break;
		default:
			// such operation is a programming error (per docs)
			// and puts the controller in a state in which it won't send an interrupt
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
	uzfx_reset(unit);

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
	bool loop_trigger = false;

	// TODO: detach zakończony EN nie jest wykonanym detachem w rozumieniu następnych komend
	// (i.e. następujące "STERUJ" zakończy się EN i nie będzie się dało z tego wyjść)
	// TODO: test
	pthread_mutex_lock(&uzfx->state_mutex);
	int old_state = uzfx->state;
	LOG(L_UZFX, "command: detach (state: %s)", uzfx_state_names[old_state]);
	switch (uzfx->state) {
		case UZFX_ST0_IDLE:
			uzfx->interrupts = UZFX_INT_NONE;
			io_ret = IO_OK;
			break;
		case UZFX_ST2_BUF_RD:
			uzfx->state = UZFX_ST2_BUF_RD_CANCEL;
			loop_trigger = true;
			io_ret = IO_EN;
			break;
		case UZFX_ST5_BUF_WR:
			uzfx->state = UZFX_ST3_BUF_WR_CANCEL;
			loop_trigger = true;
			io_ret = IO_EN;
			break;
		case UZFX_ST7_SECT_WR:
			// in normal flow, finished sector write does not send an interrupt.
			// but since we need to respond with EN here and wait for the sector
			// write to finish, make sure the interrupt is sent then
			uzfx->pending_detach_int = true;
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

	if (loop_trigger) {
		uv_async_send(&uzfx->async_work);
	}

	return io_ret;
}

// -----------------------------------------------------------------------
int uzfx_cmd(cchar_unit_t *unit, int dir, int cmd, uint16_t *r_arg)
{
	if (dir == IO_IN) {
		switch (cmd) {
			case CCHAR_CMD_SPU:
				return uzfx_cmd_spu();
			case CCHAR_CMD_READ:
				return uzfx_cmd_read(unit, r_arg);
			default:
				LOG(L_UZFX, "unknown IN command: %i", cmd);
				return IO_OK;
		}
	} else {
		switch (cmd) {
			case CCHAR_CMD_RESET:
				return uzfx_cmd_reset(unit);
			case CCHAR_CMD_DETACH:
				return uzfx_cmd_detach(unit);
			case CCHAR_CMD_WRITE:
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
