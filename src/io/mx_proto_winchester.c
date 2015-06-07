//  Copyright (c) 2013-2015 Jakub Filipowicz <jakubf@gmail.com>
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
#include <inttypes.h>
#include <arpa/inet.h>

#include "log.h"
#include "utils.h"
#include "mem/mem.h"

#include "io/mx_line.h"
#include "io/mx_irq.h"
#include "io/mx_proto.h"
#include "io/mx_proto_common.h"

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
	"[unknown]",
	"park",
	"[unknown]",
	"[unknown]"
};

// Winchester return field (state word) flags
enum mx_winch_t_status {
	MX_WS_EOF			= 0b1000000000000000,   // found end of transmission mark ($$ at the beginning of a sector)
	MX_WS_NOT_READY		= 0b0100000000000000,   // disk is not ready (no power, wrong speed, ...)
	MX_WS_ERR_WRITE		= 0b0010000000000000,   // cannot write (more than 1 head selected, no power, ...)
	MX_WS_HEADS_MOVING	= 0b0001000000000000,   // drive not ready, heads are still moving
	MX_WS_SPARE_OVLF	= 0b0000100000000000,   // spare area full (during MX_WINCH_OP_FORMAT)
	MX_WS_SPARE_MAP_ERR	= 0b0000010000000000,   // error in spare sectors map
	MX_WS_UNUSED_B6		= 0b0000001000000000,
	MX_WS_ERR			= 0b0000000100000000,   // error during processing operation, see below:
	MX_WS_BAD_SECT		= 0b0000000010000000,   // sector is marked as bad
	MX_WS_BAD_CRC		= 0b0000000001000000,   // data CRC error
	MX_WS_UNUSED_B10	= 0b0000000000100000,
	MX_WS_NO_SECTOR		= 0b0000000000010000,   // sector not found (disk address field incorrect)
	MX_WS_UNUSED_B12	= 0b0000000000001000,
	MX_WS_REJECTED		= 0b0000000000000100,   // command rejected ('cause disk is not ready)
	MX_WS_ERR_T0		= 0b0000000000000010,   // cannot position heads on track 0
	MX_WS_ERR_A1		= 0b0000000000000001	// cannot find MFM A1 data mark
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
int mx_proto_winchester_conf(struct mx_line *line, uint16_t *data)
{
	struct proto_winchester_data *proto_data = malloc(sizeof(struct proto_winchester_data));
	if (!proto_data) {
		return MX_SC_E_NOMEM;
	}

	proto_data->wide_sector_addr	=  (data[0] & 0b0000100000000000) >> 11;
	proto_data->heads				= ((data[0] & 0b0000011100000000) >> 8) + 1;
	proto_data->fprotect			=  (data[0] & 0b0000000011111111);

	LOGID(L_WNCH, 3, line, "    Winchester drive: %i heads, %s sector address%s",
		proto_data->heads,
		proto_data->wide_sector_addr ? "long" : "short",
		proto_data->fprotect ? ", format-protected" : ""
	);

	line->proto_data = proto_data;

	return MX_SC_E_OK;
}

// -----------------------------------------------------------------------
void mx_proto_winchester_free(struct mx_line *line)
{
	free(line->proto_data);
	line->proto_data = NULL;
}

// -----------------------------------------------------------------------
void mx_proto_winch_cf_decode(uint16_t *data, struct proto_winchester_data *proto_data)
{
	proto_data->op = (data[0] & 0b0000011100000000) >> 8;

	switch (proto_data->op) {
		case MX_WINCH_OP_FORMAT_SPARE:
			LOG(L_WNCH, 4, "Format spare area");
			break;
		case MX_WINCH_OP_FORMAT_TRACK:
			proto_data->format.sector_map = data[1];
			proto_data->format.start_sector = data[2] << 16;
			proto_data->format.start_sector += data[3];
			char *map = int2binf("................", data[1], 16);
			LOG(L_WNCH, 4, "Format track, starting logical sector: %i, sector map: %s", proto_data->format.start_sector, map);
			free(map);
			break;
		case MX_WINCH_OP_READ:
		case MX_WINCH_OP_WRITE:
			proto_data->transmit.ign_crc	= (data[0] & 0b0001000000000000) >> 12;
			proto_data->transmit.sector_fill= (data[0] & 0b0000100000000000) >> 11;
			proto_data->transmit.watch_eof	= (data[0] & 0b0000010000000000) >> 10;
			proto_data->transmit.cpu		= (data[0] & 0b0000000000010000) >> 4;
			proto_data->transmit.nb			= (data[0] & 0b0000000000001111) >> 0;
			proto_data->transmit.addr = data[1];
			proto_data->transmit.len = data[2]+1;
			proto_data->transmit.sector = (data[3] & 255) << 16;
			proto_data->transmit.sector += data[4];

			LOG(L_WNCH, 4, "%s %i words, starting logical sector %i, memory addres %i:0x%04x, flags: %s%s%s",
				proto_data->op == MX_WINCH_OP_READ ? "READ" : "WRITE",
				proto_data->transmit.len,
				proto_data->transmit.sector,
				proto_data->transmit.nb,
				proto_data->transmit.addr,
				proto_data->transmit.ign_crc ? "CRC_IGNORE " : "",
				proto_data->transmit.sector_fill ? "SECTOR_FILL " : "",
				proto_data->transmit.watch_eof ? "EOF_WATCH " : ""
			);
			break;
		case MX_WINCH_OP_PARK:
			proto_data->park.cylinder = data[4];
			LOG(L_WNCH, 4, "Park heads on cylinder %i", proto_data->park.cylinder);
			break;
	}

	proto_data->ret_len = 0;
	proto_data->ret_status = 0;
}

// -----------------------------------------------------------------------
void mx_proto_winch_cf_encode(uint16_t *data, struct proto_winchester_data *proto_data)
{
	data[5] = proto_data->ret_len;
	data[6] = proto_data->ret_status;
}

// -----------------------------------------------------------------------
int mx_proto_winchester_read(const struct dev_drv *dev, void *dev_obj, struct proto_winchester_data *proto)
{
	int buf_pos = 0;
	uint8_t buf[512];
	struct dev_chs chs;

	proto->ret_len = 0;

	uint16_t dest = proto->transmit.addr;

	dev_lba2chs(proto->transmit.sector, &chs, proto->heads, 16);
	// first physical cylinder is used internally by multix for relocated sectors
	chs.c++;

	// transmit data into buffer, sector by sector
	while (proto->ret_len < proto->transmit.len) {
		LOG(L_WNCH, 4, "read sector %i/%i/%i -> %i:0x%04x", chs.c, chs.h, chs.s, proto->transmit.nb, dest);

		// read the sector into buffer
		int res = dev->sector_rd(dev_obj, buf, &chs);

		// sector read OK
		if (res == DEV_CMD_OK) {
			// copy read data into system memory, swapping byte order
			buf_pos = 0;
			while ((buf_pos < 512) && (proto->ret_len < proto->transmit.len)) {
				uint16_t *buf16 = (uint16_t*)(buf + buf_pos);
				if (!mem_put(proto->transmit.nb, dest, ntohs(*buf16))) {
					return MX_IRQ_INPAO;
				}
				dest++;
				buf_pos += 2;
				(proto->ret_len)++;
			}

			dev_chs_next(&chs, proto->heads, 16);

		// sector unreadable
		} else {
			proto->ret_status = MX_WS_ERR | MX_WS_NO_SECTOR;
			return MX_IRQ_INTRA;
		}
	}

	return MX_IRQ_IETRA;
}

// -----------------------------------------------------------------------
int mx_proto_winchester_write(const struct dev_drv *dev, void *dev_obj, struct proto_winchester_data *proto)
{
	uint16_t data;
	int i;
	struct dev_chs chs;

	proto->ret_len = 0;

	uint16_t dest = proto->transmit.addr;

	dev_lba2chs(proto->transmit.sector, &chs, proto->heads, 16);
	// first physical cylinder is used internally by multix for relocated sectors
	chs.c++;

	uint8_t *buf = malloc(proto->transmit.len*2);
	if (!buf) {
		proto->ret_status = MX_WS_ERR | MX_WS_REJECTED;
		return MX_IRQ_INTRA;
	}

	// fill buffer with data to write
	for (i=0 ; i<proto->transmit.len ; i++) {
		if (!mem_get(proto->transmit.nb, dest + i, &data)) {
			free(buf);
			return MX_IRQ_INPAO;
		}
		buf[i*2] = data>>8;
		buf[i*2+1] = data&255;
	}

	// write sectors
	while (proto->ret_len < proto->transmit.len) {
		int transmit = proto->transmit.len - proto->ret_len;
		if (transmit > 256) {
			transmit = 256;
		}

		LOG(L_WNCH, 4, "write sector %i/%i/%i -> %i:0x%04x", chs.c, chs.h, chs.s, proto->transmit.nb, dest);

		int res = dev->sector_wr(dev_obj, buf + proto->ret_len*2, &chs);

		// sector not found or incomplete
		if (res != DEV_CMD_OK) {
			free(buf);
			proto->ret_status = MX_WS_ERR | MX_WS_NO_SECTOR;
			return MX_IRQ_INTRA;
		}

		dev_chs_next(&chs, proto->heads, 16);
		proto->ret_len += transmit;
	}

	free(buf);
	return MX_IRQ_IETRA;
}

// -----------------------------------------------------------------------
int mx_proto_winchester_format(const struct dev_drv *dev, void *dev_obj, struct proto_winchester_data *proto)
{
	struct dev_chs chs;
	uint8_t *buf = calloc(1, 512);

	dev_lba2chs(proto->format.start_sector, &chs, proto->heads, 16);
	chs.c++;

	for (int i=0 ; i<16 ; i++) {
		LOG(L_WNCH, 4, "format sector %i/%i/%i", chs.c, chs.h, chs.s);
		int res = dev->sector_wr(dev_obj, buf, &chs);

		// sector not found or incomplete
		if (res != DEV_CMD_OK) {
			free(buf);
			proto->ret_status = MX_WS_ERR | MX_WS_NO_SECTOR;
			return MX_IRQ_INTRA;
		}

		dev_chs_next(&chs, proto->heads, 16);
	}

	free(buf);
	return MX_IRQ_IETRA;
}

// -----------------------------------------------------------------------
uint8_t mx_proto_winchester_transmit_start(struct mx_line *line, int *irq, uint16_t *data)
{
	struct proto_winchester_data *proto_data = line->proto_data;

	// unpack control field
	mx_proto_winch_cf_decode(data, proto_data);

	// check if line is attached
	if (!(line->status & MX_LSTATE_ATTACHED)) {
		*irq = MX_IRQ_INTRA;
		goto fin;
	}

	// check if there is a device connected
	if (!line->device || !line->dev_obj) {
		proto_data->ret_status = MX_WS_NOT_READY;
		*irq = MX_IRQ_INTRA;
		goto fin;
	}

	LOGID(L_WNCH, 3, line, "Transmit operation %i: %s", proto_data->op, winch_op_names[proto_data->op]);

	switch (proto_data->op) {
		case MX_WINCH_OP_FORMAT_SPARE:
			*irq = MX_IRQ_IETRA;
			break;
		case MX_WINCH_OP_FORMAT_TRACK:
			*irq = mx_proto_winchester_format(line->device, line->dev_obj, proto_data);
			break;
		case MX_WINCH_OP_READ:
			*irq = mx_proto_winchester_read(line->device, line->dev_obj, proto_data);
			break;
		case MX_WINCH_OP_WRITE:
			*irq = mx_proto_winchester_write(line->device, line->dev_obj, proto_data);
			break;
		case MX_WINCH_OP_PARK:
			// trrrrrrrrrrrr... done.
			*irq = MX_IRQ_IETRA;
			break;
		default:
			*irq = MX_IRQ_INTRA;
			break;
	}

fin:
	// pack control field
	mx_proto_winch_cf_encode(data, proto_data);

	return MX_COND_NONE;
}

// -----------------------------------------------------------------------
uint8_t mx_proto_winchester_abort_start(struct mx_line *line, int *irq, uint16_t *data)
{
	// winchester protocol doesn't support aborting
	*irq = MX_IRQ_INABT;
	return MX_COND_NONE;
}

// -----------------------------------------------------------------------
int mx_proto_winchester_phy_types[] = { MX_PHY_WINCHESTER, -1 };

struct mx_proto mx_proto_winchester = {
	.enabled = 1,
	.name = "winchester",
	.dir = MX_DIR_NONE,
	.phy_types = mx_proto_winchester_phy_types,
	.conf = mx_proto_winchester_conf,
	.free = mx_proto_winchester_free,
	.task = {
		{ 0, 0, 1, { mx_proto_status_start, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
		{ 0, 0, 0, { mx_proto_detach_start, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
		{ 0, 0, 0, { mx_proto_oprq_start, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
		{ 5, 5, 2, { mx_proto_winchester_transmit_start, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
		{ 0, 0, 0, { mx_proto_winchester_abort_start, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
		{ 0, 0, 0, { mx_proto_attach_start, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
	}
};

// vim: tabstop=4 shiftwidth=4 autoindent
