//  Copyright (c) 2013-2018 Jakub Filipowicz <jakubf@gmail.com>
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
#include <inttypes.h>
#include <arpa/inet.h>

#include "log.h"
#include "utils/elst.h"
#include "utils/utils.h"
#include "io/mx/mx.h"
#include "io/mx/line.h"
#include "io/mx/irq.h"
#include "io/dev/dev.h"
#include "io/mx/proto_common.h"

// Transmit operations
enum mx_proto_winchester_ops {
	MX_WINCH_OP_FORMAT_SPARE	= 0,
	MX_WINCH_OP_FORMAT_TRACK	= 1,
	MX_WINCH_OP_READ			= 2,
	MX_WINCH_OP_WRITE			= 3,
	MX_WINCH_OP_PARK			= 5,
};

static const char * winch_op_names[] = {
	"format spare",
	"format",
	"read",
	"write",
	"[unknown-winch-op-4]",
	"park",
	"[unknown-winch-op-6]",
	"[unknown-winch-op-7]"
};

// Winchester return field (state word) flags
enum mx_winch_t_status {
	MX_WS_EOF			= 1 << 15,   // found end of transmission mark ($$ at the beginning of a sector)
	MX_WS_NOT_READY		= 1 << 14,   // disk is not ready (no power, wrong speed, ...)
	MX_WS_ERR_WRITE		= 1 << 13,   // cannot write (more than 1 head selected, no power, ...)
	MX_WS_HEADS_MOVING	= 1 << 12,   // drive not ready, heads are still moving
	MX_WS_SPARE_OVLF	= 1 << 11,   // spare area full (during MX_WINCH_OP_FORMAT)
	MX_WS_SPARE_MAP_ERR	= 1 << 10,   // error in spare sectors map
	MX_WS_UNUSED_B6		= 1 << 9,	// (unused)
	MX_WS_ERR			= 1 << 8,   // error during processing operation, see below:
	MX_WS_BAD_SECT		= 1 << 7,   // sector is marked as bad
	MX_WS_BAD_CRC		= 1 << 6,   // data CRC error
	MX_WS_UNUSED_B10	= 1 << 5,	// (unused)
	MX_WS_NO_SECTOR		= 1 << 4,   // sector not found (disk address field incorrect)
	MX_WS_UNUSED_B12	= 1 << 3,	// (unused)
	MX_WS_REJECTED		= 1 << 2,   // command rejected ('cause disk is not ready)
	MX_WS_ERR_T0		= 1 << 1,   // cannot position heads on track 0
	MX_WS_ERR_A1		= 1 << 0	// cannot find MFM A1 data mark
};

// transmit: format track and (optionally) move sectors to spare area
struct mx_winch_cf_format {
	uint16_t sector_map;
	unsigned start_sector;
};

// transmit: read/write
struct mx_winch_cf_transmit {
	unsigned ign_crc;
	unsigned sector_fill;
	unsigned watch_eof;
	unsigned cpu;
	unsigned nb;
	uint16_t addr;
	uint16_t len;
	unsigned sector;
};

// transmit: move heads (park)
struct mx_winch_cf_park {
	unsigned cylinder;
};

struct proto_winchester_data {
	int heads;
	int fprotect;
	int wide_sector_addr;
	unsigned op;
	struct mx_winch_cf_format format;
	struct mx_winch_cf_transmit transmit;
	struct mx_winch_cf_park park;
	uint16_t ret_len;
	uint16_t ret_status;
};

// -----------------------------------------------------------------------
int mx_winch_init(struct mx_line *pline, uint16_t *data)
{
	struct proto_winchester_data *proto_data = (struct proto_winchester_data *) malloc(sizeof(struct proto_winchester_data));
	if (!proto_data) {
		return MX_SC_E_NOMEM;
	}
	pline->proto_data = proto_data;

	// this is only used to determine how long sector address is used in spare area,
	// sectors are always addressed with 3 bytes in new MULTIX
	proto_data->wide_sector_addr	=  (data[0] & 0b0000100000000000) >> 11;
	proto_data->heads				= ((data[0] & 0b0000011100000000) >> 8) + 1;
	// this is ignored by MULTIX
	proto_data->fprotect			=  (data[0] & 0b0000000011111111);

	LOG(L_WNCH, "Winchester drive: %i heads, %s sector address%s",
		proto_data->heads,
		proto_data->wide_sector_addr ? "long" : "short",
		proto_data->fprotect ? ", format-protected" : ""
	);

	return MX_SC_E_OK;
}

// -----------------------------------------------------------------------
void mx_winch_destroy(struct mx_line *pline)
{
	if (!pline || !pline->proto_data) return;
	free(pline->proto_data);
	pline->proto_data = NULL;
}

// -----------------------------------------------------------------------
int mx_winch_trans_decode(uint16_t *data, void *proto_data)
{
	struct proto_winchester_data *pd = (struct proto_winchester_data *) proto_data;
	char map[64];

	pd->op = (data[0] & 0b0000011100000000) >> 8;

	switch (pd->op) {
		case MX_WINCH_OP_FORMAT_SPARE:
			LOG(L_WNCH, "Format spare area");
			break;
		case MX_WINCH_OP_FORMAT_TRACK:
			pd->format.sector_map = data[1];
			pd->format.start_sector = data[2] << 16;
			pd->format.start_sector += data[3];
			int2binf(map, "................", data[1], 16);
			LOG(L_WNCH, "Format track, starting logical sector: %i, sector map: %s", pd->format.start_sector, map);
			break;
		case MX_WINCH_OP_READ:
		case MX_WINCH_OP_WRITE:
			pd->transmit.ign_crc    = (data[0] & 0b0001000000000000) >> 12;
			pd->transmit.sector_fill= (data[0] & 0b0000100000000000) >> 11;
			pd->transmit.watch_eof  = (data[0] & 0b0000010000000000) >> 10;
			pd->transmit.cpu        = (data[0] & 0b0000000000010000) >> 4;
			pd->transmit.nb         = (data[0] & 0b0000000000001111);
			pd->transmit.addr       = data[1];
			pd->transmit.len        = data[2]+1;
			pd->transmit.sector     = (data[3] & 255) << 16;
			pd->transmit.sector    += data[4];

			LOG(L_WNCH, "%s %i words, starting logical sector %i, memory addres %i:0x%04x, flags: %s%s%s",
				pd->op == MX_WINCH_OP_READ ? "READ" : "WRITE",
				pd->transmit.len,
				pd->transmit.sector,
				pd->transmit.nb,
				pd->transmit.addr,
				pd->transmit.ign_crc ? "CRC_IGNORE " : "",
				pd->transmit.sector_fill ? "SECTOR_FILL " : "",
				pd->transmit.watch_eof ? "EOF_WATCH " : ""
			);
			break;
		case MX_WINCH_OP_PARK:
			pd->park.cylinder = data[4];
			LOG(L_WNCH, "Park heads on cylinder %i", pd->park.cylinder);
			break;
	}

	pd->ret_len = 0;
	pd->ret_status = 0;

	return 0;
}

// -----------------------------------------------------------------------
void mx_winch_transmit_encode(uint16_t *data, void *proto_data)
{
	struct proto_winchester_data *pd = (struct proto_winchester_data *) proto_data;

	LOG(L_WNCH, "Transmission result: len=%i, status=0x%04x", pd->ret_len, pd->ret_status);
	if ((pd->op == MX_WINCH_OP_READ) || (pd->op == MX_WINCH_OP_WRITE)) {
		data[0] = pd->ret_len;
	}
	data[1] = pd->ret_status;
}

// -----------------------------------------------------------------------
static int mx_winch_read(struct mx *multix, struct mx_line *line, const struct dev_drv *dev, void *dev_data, struct proto_winchester_data *proto_data)
{
	struct dev_chs chs;

	dev_lba2chs(proto_data->transmit.sector, &chs, proto_data->heads, 16);
	chs.c++; // first physical cylinder is used internally by multix for relocated sectors

	proto_data->ret_len = 0;
	while (proto_data->ret_len < proto_data->transmit.len) {
		// TODO: cancelation point
		int transmit = proto_data->transmit.len - proto_data->ret_len;
		if (transmit > 256) transmit = 256;

		LOG(L_WNCH, "read sector %i/%i/%i -> %i:0x%04x", chs.c, chs.h, chs.s, proto_data->transmit.nb, proto_data->transmit.addr + proto_data->ret_len);

		// read the sector into buffer
		int res = dev->sector_rd(dev_data, line->buf, &chs);

		// sector read failed
		if (res != DEV_CMD_OK) {
			proto_data->ret_status = MX_WS_ERR | MX_WS_NO_SECTOR;
			return MX_IRQ_ITRER;
		}

		// copy read data into system memory, swapping byte order
		endianswap((uint16_t*)line->buf, transmit);
		if (!mx_mem_write(multix, proto_data->transmit.nb, proto_data->transmit.addr + proto_data->ret_len, (uint16_t*)(line->buf), transmit)) {
			return MX_IRQ_INPAO;
		}

		dev_chs_next(&chs, proto_data->heads, 16); // next logical sector
		proto_data->ret_len += transmit;
	}

	return MX_IRQ_IETRA;
}

// -----------------------------------------------------------------------
static int mx_winch_write(struct mx *multix, struct mx_line *line, const struct dev_drv *dev, void *dev_data, struct proto_winchester_data *proto_data)
{   
	struct dev_chs chs;

	dev_lba2chs(proto_data->transmit.sector, &chs, proto_data->heads, 16);
	chs.c++; // first physical cylinder is used internally by multix for relocated sectors

	proto_data->ret_len = 0;
	while (proto_data->ret_len < proto_data->transmit.len) {
		// TODO: cancelation point
		int transmit = proto_data->transmit.len - proto_data->ret_len;
		if (transmit > 256) transmit = 256;

		// fill buffer with data to write
		if (!mx_mem_read(multix, proto_data->transmit.nb, proto_data->transmit.addr + proto_data->ret_len, (uint16_t*)line->buf, transmit)) {
			return MX_IRQ_INPAO;
		}
		endianswap((uint16_t*)line->buf, transmit);

		LOG(L_WNCH, "write sector %i/%i/%i <- %i:0x%04x", chs.c, chs.h, chs.s, proto_data->transmit.nb, proto_data->transmit.addr + proto_data->ret_len);

		int res = dev->sector_wr(dev_data, line->buf, &chs);

		// sector not found or incomplete
		if (res != DEV_CMD_OK) {
			proto_data->ret_status = MX_WS_ERR | MX_WS_NO_SECTOR;
			return MX_IRQ_ITRER;
		}

		dev_chs_next(&chs, proto_data->heads, 16); // next logical sector
		proto_data->ret_len += transmit;
	}

	return MX_IRQ_IETRA;
}

// -----------------------------------------------------------------------
static int mx_winch_format(struct mx *multix, struct mx_line *line, const struct dev_drv *dev, void *dev_data, struct proto_winchester_data *proto_data)
{
	struct dev_chs chs;
	memset(line->buf, '\0', MX_LINE_BUF_SIZE);

	dev_lba2chs(proto_data->format.start_sector, &chs, proto_data->heads, 16);
	chs.c++;

	for (int i=0 ; i<16 ; i++) {
		LOG(L_WNCH, "format sector %i/%i/%i", chs.c, chs.h, chs.s);
		int res = dev->sector_wr(dev_data, line->buf, &chs);

		// sector not found or incomplete
		if (res != DEV_CMD_OK) {
			proto_data->ret_status = MX_WS_ERR | MX_WS_NO_SECTOR;
			return MX_IRQ_ITRER;
		}

		dev_chs_next(&chs, proto_data->heads, 16); // next logical sector
	}

	return MX_IRQ_IETRA;
}

// -----------------------------------------------------------------------
int mx_winch_transmit(struct mx_line *line, uint16_t *cmd_data)
{
	int irq;

	struct proto_winchester_data *proto_data = (struct proto_winchester_data *) line->proto_data;

	// check if there is a device connected
	if (!line->dev || !line->dev_data) {
		proto_data->ret_len = 0;
		proto_data->ret_status = MX_WS_NOT_READY;
		irq = MX_IRQ_ITRER;
		goto fin;
	}

	LOG(L_WNCH, "Transmit operation %i: %s", proto_data->op, winch_op_names[proto_data->op]);

	switch (proto_data->op) {
		case MX_WINCH_OP_FORMAT_SPARE:
			LOG(L_WNCH, "Formatting spare area (unhandled)");
			// TODO: em400 does not support spare area
			irq = MX_IRQ_IETRA;
			break;
		case MX_WINCH_OP_FORMAT_TRACK:
			irq = mx_winch_format(line->multix, line, line->dev, line->dev_data, proto_data);
			break;
		case MX_WINCH_OP_READ:
			irq = mx_winch_read(line->multix, line, line->dev, line->dev_data, proto_data);
			break;
		case MX_WINCH_OP_WRITE:
			irq = mx_winch_write(line->multix, line, line->dev, line->dev_data, proto_data);
			break;
		case MX_WINCH_OP_PARK:
			LOG(L_WNCH, "Parking heads on cylinder %i (unhandled)", proto_data->park.cylinder);
			// trrrrrrrrrrrr... done.
			irq = MX_IRQ_IETRA;
			break;
		default:
			irq = MX_IRQ_INTRA;
			break;
	}

fin:
	return irq;
}

// -----------------------------------------------------------------------
const struct mx_proto mx_drv_winchester = {
	.name = "winchester",
	.dir = MX_DIR_NONE,
	.phy_types = { MX_PHY_WINCHESTER, -1 },
	.init = mx_winch_init,
	.destroy = mx_winch_destroy,
	.cmd = {
		[MX_CMD_ATTACH] = { 0, 0, NULL, NULL, mx_dummy_attach },
		[MX_CMD_TRANSMIT] = { 5, 2, mx_winch_trans_decode, mx_winch_transmit_encode, mx_winch_transmit },
		[MX_CMD_DETACH] = { 0, 0, NULL, NULL, mx_dummy_detach },
		[MX_CMD_ABORT] = { 0, 0, NULL, NULL, NULL },
	}
};

// vim: tabstop=4 shiftwidth=4 autoindent
