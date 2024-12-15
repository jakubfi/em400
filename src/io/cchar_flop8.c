//  Copyright (c) 2013 Jakub Filipowicz <jakubf@gmail.com>
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
#include <stdatomic.h>

#include "em400.h"
#include "io/defs.h"
#include "io/cchar.h"
#include "io/cchar_flop8.h"
#include "cfg.h"

#include "log.h"

#define F8_DRIVE_CNT 4
#define F8_TRACK_CNT 77
#define F8_TRACK_LAST 73
#define F8_SECTOR_PER_TRACK 26
#define F8_BYTES_PER_SECTOR 128
#define F8_DISK_SIZE_BYTES (F8_TRACK_CNT * F8_SECTOR_PER_TRACK * F8_BYTES_PER_SECTOR)

enum f8_states {
	F8ST_IDLE,					// St.0 idle state
	F8ST_SECT_RD,				// St.1 disk to buffer read
	F8ST_SECT_RD_CANCEL,		// cancel reading current sector
	F8ST_BUF_RD,				// St.2 buffer to cpu read
	F8ST_BUF_WR_INIT,			// first state after buffer write (prepare for write)
	F8ST_BUF_WR,				// cpu to buffer write St.3 (bad sector mark, F8) St.5 (regular data FB)
	F8ST_SECT_WR,				// buffer to disk write St.6 (with check), St.7 (no check)
	F8ST_QUIT,
};

enum f8_commands {
	F8CMD_QUIT		= -1,
	// OU
	F8CMD_RESET		= 0b100000, // reset
	F8CMD_DETACH	= 0b101000, // detach (soft reset)
	F8CMD_WRITE		= 0b110000, // write
	F8CMD_CTL_W		= 0b111000, // control for write
	F8CMD_CTL_WC	= 0b111100, // control for write+check
	F8CMD_CTL_B		= 0b111110, // control for bad sector
	F8CMD_CTL_R		= 0b111010, // control for read
	// IN
	F8CMD_SPU		= 0b100000, // check device
	F8CMD_READ		= 0b101000, // read
};

#define F8_INT_NONE 0
enum f8_interrupt_priorities {
	F8_INT_SECT_NOT_FOUND, // highest prio
	F8_INT_CRC_ERR,
	F8_INT_SECT_BAD,
	F8_INT_HW_ERR,
	F8_INT_DISK_END,
	F8_INT_READY, // lowest prio
};
static const int f8_interrupt_specs[] = {
	0b11010, // sector not found
	0b01010, // data CRC error
	0b10010, // sector marked as bad
	0b00010, // hardware error
	0b00100, // track==73 && sector==26
	0b00001, // device is ready
};

enum f8_drives {
	F8_DRV_0	= 0b000 << 13,
	F8_DRV_1	= 0b001 << 13,
	F8_DRV_2	= 0b100 << 13,
	F8_DRV_3	= 0b101 << 13,
};

enum f8_sides {
	F8_SIDE_A	= 0 << 12,
	F8_SIDE_B	= 1 << 12,
};

#define F8_ADDR_TRACK(n) (n << 5)
#define F8_ADDR_SECTOR(n) (n)

#define INITIAL_ADDRESS (F8_DRV_0 | F8_SIDE_A | F8_ADDR_TRACK(1) | F8_ADDR_SECTOR(1))

typedef struct flop8 {
	struct cchar_unit_proto_t proto;
	char *image[F8_DRIVE_CNT];
	FILE *f[F8_DRIVE_CNT];
	int drive, side, track, sector;
	int buf_pos;
	uint8_t buf[F8_BYTES_PER_SECTOR];
	pthread_t worker;
	pthread_mutex_t state_mutex;
	pthread_cond_t state_cond;
	int state;
	int operation;
	bool pending_write;
	bool force_interrupt;
	int interrupts;
} flop8;

static void * flop8_worker_loop(void *ptr);
static void flop8_reset_state(flop8 *u);

// -----------------------------------------------------------------------
static void flop8_set_address(flop8 *u, uint16_t addr)
{
	int drive = (addr >> 13) & 0b111;
	u->drive = (drive & 1) | ((drive >> 1) & 2);
	u->side = (addr >> 12) & 1;
	u->track = (addr >> 5) & 0b1111111;
	u->sector = addr & 0b11111;
	u->buf_pos = 0;
}

// -----------------------------------------------------------------------
struct cchar_unit_proto_t * cchar_flop8_create(em400_cfg *cfg, int ch_num, int dev_num)
{
	flop8 *flop = (flop8 *) calloc(1, sizeof(flop8));
	if (!flop) {
		LOGERR("Failed to allocate memory for 8-inch floppy: %i.%i", ch_num, dev_num);
		goto fail;
	}

	for (int id=0 ; id<F8_DRIVE_CNT ; id++) {
		const char *image = cfg_fgetstr(cfg, "dev%i.%i:image_%i", ch_num, dev_num, id);
		if (!image) continue;

		flop->image[id] = strdup(image);
		if (!flop->image[id]) {
			LOGERR("8-inch floppy image data memory allocation error.");
			goto fail;
		}
		flop->f[id] = fopen(image, "r+b");
		if (flop->f[id]) {
			LOG(L_FLOP, "Drive %i: %s.", id, image);
		} else {
			LOGERR("Failed to open 8-inch floppy image: %s (drive %i).", image, id);
			goto fail;
		}

		fseek(flop->f[id], 0L, SEEK_END);
		int sz = ftell(flop->f[id]);
		if (sz != F8_DISK_SIZE_BYTES) {
			LOGERR("Wrong 8-inch floppy drive %i image '%s' size: %i instead of %i.", id, image, sz, F8_DISK_SIZE_BYTES);
			goto fail;
		}
	}

	if (pthread_mutex_init(&flop->state_mutex, NULL)) {
		LOGERR("Failed to initialize 8-inch floppy status mutex.");
		goto fail;
	}

	if (pthread_cond_init(&flop->state_cond, NULL)) {
		LOGERR("Failed to initialize 8-inch floppy status cond.");
		goto fail;
	}

	if (pthread_create(&flop->worker, NULL, flop8_worker_loop, flop)) {
		LOGERR("Failed to spawn 8-inch floppy worker thread.");
		goto fail;
	}

	flop8_reset_state(flop);

	return (struct cchar_unit_proto_t *) flop;

fail:
	cchar_flop8_shutdown((struct cchar_unit_proto_t*) flop);
	return NULL;
}

// -----------------------------------------------------------------------
static void flop8_reset_state(flop8 *flop)
{
	pthread_mutex_lock(&flop->state_mutex);
	flop8_set_address(flop, INITIAL_ADDRESS);
	flop->state = F8ST_IDLE;
	flop->pending_write = false;
	flop->force_interrupt = false;
	atomic_store_explicit(&flop->interrupts, F8_INT_NONE, memory_order_release);
	pthread_mutex_unlock(&flop->state_mutex);
}

// -----------------------------------------------------------------------
void cchar_flop8_shutdown(struct cchar_unit_proto_t *unit)
{
	flop8 *flop = (flop8 *) unit;
	if (!flop) return;

	pthread_mutex_lock(&flop->state_mutex);
	flop->state = F8ST_QUIT;
	pthread_cond_signal(&flop->state_cond);
	pthread_mutex_unlock(&flop->state_mutex);

	if (flop->worker) pthread_join(flop->worker, NULL);
	pthread_mutex_destroy(&flop->state_mutex);
	pthread_cond_destroy(&flop->state_cond);

	for (int i=0 ; i<F8_DRIVE_CNT ; i++) {
		if (flop->f[i]) fclose(flop->f[i]);
		if (flop->image[i]) free(flop->image[i]);
	}
	free(flop);
}

// -----------------------------------------------------------------------
void cchar_flop8_reset(struct cchar_unit_proto_t *unit)
{
	flop8 *flop = (flop8 *) unit;
	LOG(L_FLOP, "reset");
	flop8_reset_state(flop);
	cchar_int_cancel(flop->proto.chan, flop->proto.num);
	LOG(L_FLOP, "reset done");
}

// -----------------------------------------------------------------------
static bool f8_sector_advance(flop8 *flop)
{
	flop->sector++;
	flop->buf_pos = 0;

	LOG(L_FLOP, "next sector: %i", flop->sector);

	if (flop->sector > F8_SECTOR_PER_TRACK) {
		flop->sector = 1;
		flop->track++;
		LOG(L_FLOP, "track end, next track: %i", flop->track);
		if (flop->track > F8_TRACK_LAST) {
			flop->track = 1;
			LOG(L_FLOP, "disk end");
			return true;
		}
	}
	return false;
}

// -----------------------------------------------------------------------
static bool f8_img_seek(flop8 *flop)
{
	pthread_mutex_lock(&flop->state_mutex);
	LOG(L_FLOP, "Seek: track %i, sector %i", flop->track, flop->sector);
	int offset = (flop->track * F8_SECTOR_PER_TRACK + (flop->sector-1)) * F8_BYTES_PER_SECTOR;
	pthread_mutex_unlock(&flop->state_mutex);
	int res = fseek(flop->f[flop->drive], offset, SEEK_SET);
	if (res != 0) {
		LOG(L_FLOP, "Image file seek failed");
		return true;
	}
	return false;
}

// -----------------------------------------------------------------------
static bool f8_img_read_sector(flop8 *flop)
{
	pthread_mutex_lock(&flop->state_mutex);
	int res = fread(&flop->buf, 1, F8_BYTES_PER_SECTOR, flop->f[flop->drive]);
	if (res != F8_BYTES_PER_SECTOR) {
		LOG(L_FLOP, "Failed floppy read");
		return true;
	}
	pthread_mutex_unlock(&flop->state_mutex);
	LOG(L_FLOP, "Read sector (%x %x %x ...)", flop->buf[0], flop->buf[1], flop->buf[2]);
	return false;
}

// -----------------------------------------------------------------------
static bool f8_img_write_sector(flop8 *flop)
{
	LOG(L_FLOP, "Write sector (%x %x %x ...)", flop->buf[0], flop->buf[1], flop->buf[2]);
	int res = fwrite(&flop->buf, 1, F8_BYTES_PER_SECTOR, flop->f[flop->drive]);
	if (res != F8_BYTES_PER_SECTOR) {
		LOG(L_FLOP, "Failed floppy write");
		return true;
	}
	return false;
}

// -----------------------------------------------------------------------
static void * flop8_worker_loop(void *ptr)
{
	flop8 *flop = (flop8 *) ptr;
	int interrupt;
	int state;
	bool quit = false;

	while (!quit) {

		pthread_mutex_lock(&flop->state_mutex);
		// last two states are not handled here
		// TODO: flop FSM... and multithreading...
		while ((flop->state == F8ST_IDLE) || (flop->state == F8ST_BUF_RD) || (flop->state == F8ST_BUF_WR)) {
			LOG(L_FLOP, "Worker waiting for state change");
			pthread_cond_wait(&flop->state_cond, &flop->state_mutex);
		}
		interrupt = F8_INT_NONE;
		state = flop->state;
		pthread_mutex_unlock(&flop->state_mutex);

		switch (state) {
			case F8ST_QUIT:
				LOG(L_FLOP, "Worker processing state: QUIT");
				quit = true;
				break;
			case F8ST_SECT_RD:
				LOG(L_FLOP, "Worker processing state: SECTOR READ");
				if (!flop->f[flop->drive]) {
					LOG(L_FLOP, "No image attached to drive %i", flop->drive);
					interrupt |= 1 << F8_INT_HW_ERR;
					pthread_mutex_lock(&flop->state_mutex);
					flop->state = F8ST_IDLE;
					pthread_mutex_unlock(&flop->state_mutex);
				} else {
					if ((flop->sector == 26) && (flop->track == 73)) {
						interrupt |= 1 << F8_INT_DISK_END;
					}
					f8_img_seek(flop);
					f8_img_read_sector(flop);
					interrupt |= 1 << F8_INT_READY;
					pthread_mutex_lock(&flop->state_mutex);
					flop->buf_pos = 0;
					flop->state = F8ST_BUF_RD;
					pthread_mutex_unlock(&flop->state_mutex);
				}
				break;
			case F8ST_SECT_RD_CANCEL:
				LOG(L_FLOP, "Worker processing state: SECTOR READ CANCEL");
				f8_sector_advance(flop);
				interrupt |= 1 << F8_INT_READY;
				pthread_mutex_lock(&flop->state_mutex);
				flop->state = F8ST_IDLE;
				pthread_mutex_unlock(&flop->state_mutex);
				break;
			case F8ST_BUF_WR_INIT:
				LOG(L_FLOP, "Worker processing state: WRITE INIT");
				if (!flop->f[flop->drive]) {
					LOG(L_FLOP, "No image attached to drive %i", flop->drive);
					interrupt |= 1 << F8_INT_HW_ERR;
					pthread_mutex_lock(&flop->state_mutex);
					flop->state = F8ST_IDLE;
					pthread_mutex_unlock(&flop->state_mutex);
				} else {
					// TODO: start the engine
					// TODO: ustalenie sposobu/rodzaju zapisu
					if ((flop->sector == 26) && (flop->track == 73)) {
						interrupt |= 1 << F8_INT_DISK_END;
					}
					interrupt |= 1 << F8_INT_READY;
					pthread_mutex_lock(&flop->state_mutex);
					flop->state = F8ST_BUF_WR;
					flop->buf_pos = 0;
					pthread_mutex_unlock(&flop->state_mutex);
				}
				break;
			case F8ST_SECT_WR:
				LOG(L_FLOP, "Worker processing state: SECTOR WRITE");
				f8_img_seek(flop);
				f8_img_write_sector(flop);
				f8_sector_advance(flop);
				// TODO: jeśli z kontrolą - odczyt
				pthread_mutex_lock(&flop->state_mutex);
				if (flop->force_interrupt) {
					interrupt |= 1 << F8_INT_READY;
				}
				if (flop->pending_write) {
					flop->state = F8ST_BUF_WR_INIT;
					flop->pending_write = false;
				} else {
					flop->state = F8ST_IDLE;
				}
				pthread_mutex_unlock(&flop->state_mutex);
				break;
		}

		if (interrupt != F8_INT_NONE) {
			LOG(L_FLOP, "Worker sending interrupt");
			atomic_store_explicit(&flop->interrupts, interrupt, memory_order_release);
			cchar_int_trigger(flop->proto.chan);
			flop->force_interrupt = false;
		}

	}

	LOG(L_FLOP, "Leaving 8-inch floppy worker loop");
	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
bool cchar_flop8_has_interrupt(struct cchar_unit_proto_t *unit)
{
	flop8 *flop = (flop8 *) unit;
	return atomic_load_explicit(&flop->interrupts, memory_order_acquire) ? true : false;
}

// -----------------------------------------------------------------------
int cchar_flop8_intspec(struct cchar_unit_proto_t *unit)
{
	flop8 *flop = (flop8 *) unit;

	int spec = F8_INT_NONE;
	int ints = atomic_load_explicit(&flop->interrupts, memory_order_acquire);

	for (int shift=0 ; shift<=5 ; shift++) {
		if (ints & (1<<shift)) {
			spec = f8_interrupt_specs[shift];
			atomic_store_explicit(&flop->interrupts, ints & ~(1<<shift), memory_order_release);
			break;
		}
	}

	return spec;
}

// -----------------------------------------------------------------------
static int f8_cmd_read(struct cchar_unit_proto_t *unit, uint16_t *r_arg)
{
	flop8 *flop = (flop8 *) unit;
	int io_ret;

	pthread_mutex_lock(&flop->state_mutex);
	switch (flop->state) {
		case F8ST_IDLE:
			LOG(L_FLOP, "command: read (state: idle)");
			flop->state = F8ST_SECT_RD;
			pthread_cond_signal(&flop->state_cond);
			io_ret = IO_EN;
			break;
		case F8ST_BUF_RD:
			*r_arg = flop->buf[flop->buf_pos];
			LOG(L_FLOP, "command: read byte 0x%02x (state: buffer read)", *r_arg);
			flop->buf_pos++;
			if (flop->buf_pos >= F8_BYTES_PER_SECTOR) {
				// last byte, set next sector address
				f8_sector_advance(flop);
				flop->state = F8ST_IDLE;
			}
			atomic_store_explicit(&flop->interrupts, F8_INT_NONE, memory_order_release);
			io_ret = IO_OK;
			break;
		default:
			LOG(L_FLOP, "command: read (state: other)");
			io_ret = IO_EN;
			break;
	}
	pthread_mutex_unlock(&flop->state_mutex);

	return io_ret;
}

// -----------------------------------------------------------------------
static int f8_cmd_write(struct cchar_unit_proto_t *unit, uint16_t *r_arg)
{
	flop8 *flop = (flop8 *) unit;
	int io_ret;

	pthread_mutex_lock(&flop->state_mutex);
	switch (flop->state) {
		case F8ST_IDLE:
			LOG(L_FLOP, "command: write (state: idle)");
			flop->state = F8ST_BUF_WR_INIT;
			pthread_cond_signal(&flop->state_cond);
			io_ret = IO_EN;
			break;
		case F8ST_BUF_WR:
			flop->buf[flop->buf_pos] = *r_arg;
			LOG(L_FLOP, "command: write byte 0x%02x (state: buffer write)", flop->buf[flop->buf_pos]);
			flop->buf_pos++;
			if (flop->buf_pos >= F8_BYTES_PER_SECTOR) {
				flop->state = F8ST_SECT_WR;
				pthread_cond_signal(&flop->state_cond);
			}
			atomic_store_explicit(&flop->interrupts, F8_INT_NONE, memory_order_release);
			io_ret = IO_OK;
			break;
		case F8ST_SECT_WR:
			LOG(L_FLOP, "command: write (state: sector write)");
			flop->pending_write = true;
			pthread_cond_signal(&flop->state_cond);
			io_ret = IO_EN;
			break;
		default:
			LOG(L_FLOP, "command: write (state: other)");
			io_ret = IO_EN;
			break;
	}
	pthread_mutex_unlock(&flop->state_mutex);

	return io_ret;
}

// -----------------------------------------------------------------------
static int f8_cmd_control(struct cchar_unit_proto_t *unit, uint16_t *r_arg, int cmd)
{
	flop8 *flop = (flop8 *) unit;
	int io_ret;

	pthread_mutex_lock(&flop->state_mutex);
	switch (flop->state) {
		case F8ST_IDLE:
			LOG(L_FLOP, "command: control (state: idle)");
			flop->operation = cmd;
			flop8_set_address(flop, *r_arg);
			LOG(L_FLOP, "Set new address: drive %i, side: %i, track %i, sector %i", flop->drive, flop->side, flop->track, flop->sector);
			io_ret = IO_OK;
			break;
		default:
			LOG(L_FLOP, "command: control (state: other)");
			io_ret = IO_EN;
			break;
	}
	pthread_mutex_unlock(&flop->state_mutex);

	return io_ret;
}

// -----------------------------------------------------------------------
static int f8_cmd_reset(struct cchar_unit_proto_t *unit)
{
	LOG(L_FLOP, "command: reset");
	flop8 *flop = (flop8*) unit;
	flop8_reset_state(flop);
	cchar_int_cancel(flop->proto.chan, flop->proto.num);

	return IO_OK;
}

// -----------------------------------------------------------------------
static int f8_cmd_spu()
{
	LOG(L_FLOP, "command: SPU");

	return IO_OK;
}

// -----------------------------------------------------------------------
static int f8_cmd_detach(struct cchar_unit_proto_t *unit)
{
	LOG(L_FLOP, "command: detach");
	flop8 *flop = (flop8 *) unit;
	int io_ret;

	pthread_mutex_lock(&flop->state_mutex);
	switch (flop->state) {
		case F8ST_IDLE:
			LOG(L_FLOP, "command: detach (state: idle)");
			atomic_store_explicit(&flop->interrupts, F8_INT_NONE, memory_order_release);
			io_ret = IO_OK;
			break;
		case F8ST_BUF_RD:
			LOG(L_FLOP, "command: detach (state: buffer read)");
			flop->state = F8ST_SECT_RD_CANCEL;
			pthread_cond_signal(&flop->state_cond);
			io_ret = IO_EN;
			break;
		case F8ST_BUF_WR:
			LOG(L_FLOP, "command: detach (state: buffer write)");
			while (flop->buf_pos < F8_BYTES_PER_SECTOR) {
				flop->buf[flop->buf_pos] = 0;
				flop->buf_pos++;
			}
			flop->state = F8ST_SECT_WR;
			flop->force_interrupt = true;
			pthread_cond_signal(&flop->state_cond);
			io_ret = IO_EN;
			break;
		case F8ST_SECT_WR:
			LOG(L_FLOP, "command: detach (state: sector write)");
			flop->force_interrupt = true;
			flop->pending_write = false;
			io_ret = IO_EN;
			break;
		default:
			LOG(L_FLOP, "command: detach (state: other)");
			io_ret = IO_EN;
			break;
	}
	pthread_mutex_unlock(&flop->state_mutex);

	return io_ret;
}

// -----------------------------------------------------------------------
int cchar_flop8_cmd(struct cchar_unit_proto_t *unit, int dir, int cmd, uint16_t *r_arg)
{
	if (dir == IO_IN) {
		switch (cmd) {
			case F8CMD_SPU:
				return f8_cmd_spu();
			case F8CMD_READ:
				return f8_cmd_read(unit, r_arg);
			default:
				LOG(L_FLOP, "unknown IN command: %i", cmd);
				return IO_OK;
		}
	} else {
		switch (cmd) {
			case F8CMD_RESET:
				return f8_cmd_reset(unit);
			case F8CMD_DETACH:
				return f8_cmd_detach(unit);
			case F8CMD_WRITE:
				return f8_cmd_write(unit, r_arg);
			case F8CMD_CTL_R:
			case F8CMD_CTL_W:
			case F8CMD_CTL_WC:
			case F8CMD_CTL_B:
				return f8_cmd_control(unit, r_arg, cmd);
			default:
				LOG(L_FLOP, "unknown OUT command: %i", cmd);
				return IO_OK;
		}
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
