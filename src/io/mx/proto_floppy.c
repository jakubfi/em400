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

#include "io/mx/mx.h"
#include "io/mx/line.h"
#include "io/mx/irq.h"

#include "log.h"

enum mx_floppy_types {
	MX_FLOPPY_SD,
	MX_FLOPPY_DD,
	MX_FLOPPY_HD,
	MX_FLOPPY_MAX,
};

static const char *mx_floppy_type_names[] = {
	"SD",
	"DD",
	"DD/HD",
	"[unknown-floppy-drv-type]",
};

enum mx_proto_floppy_ops {
	MX_FLOPPY_OP_0,
	MX_FLOPPY_OP_FORMAT,
	MX_FLOPPY_OP_READ,
	MX_FLOPPY_OP_WRITE,
	MX_FLOPPY_OP_ERRSECT,
	MX_FLOPPY_OP_CNT
};

static const char * floppy_op_names[] = {
	"[unknown-0]",
	"format",
	"read",
	"write",
	"sect. error write",
	"[unknown]"
};

enum mx_floppy_t_status {
	// first status word
	// oo........o..ooo .oo..... 00000000
	MX_FS_NO_DISC =     1 << 31, // no disc inserted, hardware failure
	MX_FS_ERR =         1 << 30, // transmission error (more details below)
	MX_FS_CRC_DA =      1 << 21, // data/address CRC error
	MX_FS_ADDR =        1 << 18, // address field not found
	MX_FS_WRPROT =      1 << 17, // media write protected
	MX_FS_NO_DATA =     1 << 16, // no data (?)
	// second status byte
	MX_FS_ERR_SECT =    1 << 14, // bad sector marker found
	MX_FS_CRC_D =       1 << 13, // data CRC error
};

struct proto_floppy_data {
	int type;
	int fprotect;
	int retrans;
	int spt;
	int slen;
	int cyls;
	int heads;
	int s1num;
	unsigned op;
	int ign_crc;
	int nb;
	uint16_t addr;
	int len;
	int sect;
	uint16_t ret_len;
	uint32_t ret_status;
};

// -----------------------------------------------------------------------
static const char * mx_proto_floppy_get_type_name(unsigned i)
{
	if (i >= MX_FLOPPY_MAX) {
		i = MX_FLOPPY_MAX;
	}
	return mx_floppy_type_names[i];
}

// -----------------------------------------------------------------------
int mx_floppy_init(struct mx_line *pline, uint16_t *data)
{
	struct proto_floppy_data *proto_data = malloc(sizeof(struct proto_floppy_data));
	if (!proto_data) {
		return MX_SC_E_NOMEM;
	}

	proto_data->type	 = (data[0] & 0b1111111100000000) >> 8;
	proto_data->fprotect = (data[0] & 0b0000000011111111);

	LOG(L_FLOP, 3, "    %s floppy drive%s",
		mx_proto_floppy_get_type_name(proto_data->type),
		proto_data->fprotect ? ", format-protected" : ""
	);

	if (proto_data->type >= MX_FLOPPY_MAX) {
		free(proto_data);
		return MX_SC_E_PROTO_PARAMS;
	}

	pline->proto_data = proto_data;

	return MX_SC_E_OK;
}

// -----------------------------------------------------------------------
void mx_floppy_destroy(struct mx_line *pline)
{
	if (!pline || !pline->proto_data) return;
	free(pline->proto_data);
	pline->proto_data = NULL;
}

// -----------------------------------------------------------------------
static int mx_floppy_att_decode(uint16_t *data, struct proto_floppy_data *proto_data)
{
	proto_data->retrans = (data[0] & 0b0000111100000000) >> 8;
	proto_data->spt =     (data[0] & 0b0000000011111111) >> 0;
	unsigned slen =       (data[1] & 0b1111111100000000) >> 8;
	proto_data->cyls =    (data[1] & 0b0000000011111111) >> 0;
	proto_data->heads =   (data[2] & 0b1111111100000000) >> 8;
	proto_data->s1num =   (data[2] & 0b0000000011111111) >> 0;

	if (slen > 3) {
		LOG(L_FLOP, 4, "Attaching floppy: wrong number of words per sector selection: %i", slen);
		return -1;
	}

	proto_data->slen = 64 * (1 << slen);

	LOG(L_FLOP, 4, "Attaching floppy: %i heads, %i cyls, %i words per sector, %i sectors per track, %i retransmissions, first sector = %i",
		proto_data->heads,
		proto_data->cyls,
		proto_data->slen,
		proto_data->spt,
		proto_data->retrans,
		proto_data->s1num
	);
	return 0;
}

// -----------------------------------------------------------------------
static int mx_floppy_transmit_decode(uint16_t *data, struct proto_floppy_data *proto_data)
{
	proto_data->op =  (data[0] & 0b0000011100000000) >> 8;
	proto_data->sect = data[3];

	switch (proto_data->op) {
		case MX_FLOPPY_OP_FORMAT:
			LOG(L_FLOP, 4, "Format track, starting sector: %i", proto_data->sect);
			break;
		case MX_FLOPPY_OP_READ:
		case MX_FLOPPY_OP_WRITE:
			proto_data->ign_crc = (data[0] & 0b0001000000000000) >> 12;
			proto_data->nb =      (data[0] & 0b0000000000011111) >> 0;
			proto_data->addr =     data[1];
			proto_data->len =      data[2] + 1;
			LOG(L_FLOP, 4, "%s %i words, starting sector: %i, memory address: %i:0x%04x %s",
				(proto_data->op == MX_FLOPPY_OP_READ) ? "Read" : "Write",
				proto_data->len,
				proto_data->sect,
				proto_data->nb,
				proto_data->addr,
				proto_data->ign_crc ? "(ignore CRC)" : ""
			);
			break;
		case MX_FLOPPY_OP_ERRSECT:
			LOG(L_FLOP, 4, "Mark sector %i as bad", proto_data->sect);
			break;
		default:
			return -1;
			break;
	}

	return 0;
}

// -----------------------------------------------------------------------
static void mx_floppy_transmit_encode(uint16_t *data, struct proto_floppy_data *proto_data)
{
	LOG(L_FLOP, 4, "Transmit status: ret_len=0x%04x, ret_status=0x%08x", proto_data->ret_len, proto_data->ret_status);
	if ((proto_data->op == MX_FLOPPY_OP_READ) || (proto_data->op == MX_FLOPPY_OP_WRITE)) {
		data[0] = proto_data->ret_len;
	}
	data[1] = proto_data->ret_status >> 16;
	data[2] = proto_data->ret_status & 0xffff;
}

// -----------------------------------------------------------------------
int mx_floppy_attach(struct mx_line *lline)
{
	int irq;
	struct proto_floppy_data *proto_data = lline->proto_data;

	if (mx_floppy_att_decode(lline->cmd_data, proto_data)) {
		irq = MX_IRQ_INDOL;
	} else {
		pthread_mutex_lock(&lline->status_mutex);
		lline->status |= MX_LSTATE_ATTACHED;
		pthread_mutex_unlock(&lline->status_mutex);
		irq = MX_IRQ_INDOL;
	}

	return irq;
}

// -----------------------------------------------------------------------
int mx_floppy_detach(struct mx_line *lline)
{ 
	pthread_mutex_lock(&lline->status_mutex);
	lline->status &= ~MX_LSTATE_ATTACHED;
	pthread_mutex_unlock(&lline->status_mutex);

	return MX_IRQ_IODLI;
}

// -----------------------------------------------------------------------
int mx_floppy_abort(struct mx_line *lline)
{ 
	return MX_IRQ_INABT;
}

// -----------------------------------------------------------------------
int mx_floppy_transmit(struct mx_line *lline)
{
	int irq;
	struct proto_floppy_data *proto_data = lline->proto_data;
	const struct mx_cmd *cmd = lline->proto->cmd + MX_CMD_TRANSMIT;

	if (mx_floppy_transmit_decode(lline->cmd_data, proto_data)) {
		irq = MX_IRQ_INTRA;
		goto fin;
	}

	// check if there is a device connected
	if (!lline->dev || !lline->dev_data) {
		proto_data->ret_status = MX_FS_NO_DISC;
		irq = MX_IRQ_INTRA;
		goto fin;
	}

	LOG(L_FLOP, 3, "Transmit operation %i: %s", proto_data->op, floppy_op_names[proto_data->op]);

	// TODO: temporary, so mega bootloader works
	proto_data->ret_status = MX_FS_NO_DISC;
	irq = MX_IRQ_ITRER;

fin:
	// pack control field
	mx_floppy_transmit_encode(lline->cmd_data + cmd->output_fpos, proto_data);

	return irq;
}

// -----------------------------------------------------------------------
const struct mx_proto mx_drv_floppy = {
	.name = "floppy",
	.dir = MX_DIR_NONE,
	.phy_types = { MX_PHY_FLOPPY, -1 },
	.init = mx_floppy_init,
	.destroy = mx_floppy_destroy,
	.cmd = {
		[MX_CMD_ATTACH] = { 3, 0, 0, mx_floppy_attach },
		[MX_CMD_TRANSMIT] = { 4, 4, 3, mx_floppy_transmit },
		[MX_CMD_DETACH] = { 0, 0, 0, mx_floppy_detach },
		[MX_CMD_ABORT] = { 0, 0, 0, mx_floppy_abort },
	}
};

// vim: tabstop=4 shiftwidth=4 autoindent
