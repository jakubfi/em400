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
	"[unknown-floppy-op-0]",
	"format",
	"read",
	"write",
	"sect. error write",
	"[unknown-floppy-op]"
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

	LOG(L_FLOP, "%s floppy drive%s",
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
int mx_floppy_att_decode(uint16_t *data, void *proto_data)
{
	struct proto_floppy_data *pd = proto_data;

	pd->retrans =  (data[0] & 0b0000111100000000) >> 8;
	pd->spt =      (data[0] & 0b0000000011111111) >> 0;
	unsigned slen =(data[1] & 0b1111111100000000) >> 8;
	pd->cyls =     (data[1] & 0b0000000011111111) >> 0;
	pd->heads =    (data[2] & 0b1111111100000000) >> 8;
	pd->s1num =    (data[2] & 0b0000000011111111) >> 0;

	if (slen > 3) {
		LOG(L_FLOP, "Attaching floppy: wrong number of words per sector selection: %i", slen);
		return -1;
	}

	pd->slen = 64 * (1 << slen);

	LOG(L_FLOP, "Attaching floppy: %i heads, %i cyls, %i words per sector, %i sectors per track, %i retransmissions, first sector = %i",
		pd->heads,
		pd->cyls,
		pd->slen,
		pd->spt,
		pd->retrans,
		pd->s1num
	);

	return 0;
}

// -----------------------------------------------------------------------
int mx_floppy_transmit_decode(uint16_t *data, void *proto_data)
{
	struct proto_floppy_data *pd = proto_data;

	pd->op =  (data[0] & 0b0000011100000000) >> 8;
	pd->sect = data[3];

	switch (pd->op) {
		case MX_FLOPPY_OP_FORMAT:
			LOG(L_FLOP, "Format track, starting sector: %i", pd->sect);
			break;
		case MX_FLOPPY_OP_READ:
		case MX_FLOPPY_OP_WRITE:
			pd->ign_crc = (data[0] & 0b0001000000000000) >> 12;
			pd->nb =      (data[0] & 0b0000000000011111) >> 0;
			pd->addr =     data[1];
			pd->len =      data[2] + 1;
			LOG(L_FLOP, "%s %i words, starting sector: %i, memory address: %i:0x%04x %s",
				(pd->op == MX_FLOPPY_OP_READ) ? "Read" : "Write",
				pd->len,
				pd->sect,
				pd->nb,
				pd->addr,
				pd->ign_crc ? "(ignore CRC)" : ""
			);
			break;
		case MX_FLOPPY_OP_ERRSECT:
			LOG(L_FLOP, "Mark sector %i as bad", pd->sect);
			break;
		default:
			return -1;
			break;
	}

	return 0;
}

// -----------------------------------------------------------------------
void mx_floppy_transmit_encode(uint16_t *data, void *proto_data)
{
	struct proto_floppy_data *pd = proto_data;

	LOG(L_FLOP, "Transmission result: len=%i, status=0x%08x", pd->ret_len, pd->ret_status);
	if ((pd->op == MX_FLOPPY_OP_READ) || (pd->op == MX_FLOPPY_OP_WRITE)) {
		data[0] = pd->ret_len;
	}
	data[1] = pd->ret_status >> 16;
	data[2] = pd->ret_status & 0xffff;
}

// -----------------------------------------------------------------------
int mx_floppy_attach(struct mx_line *lline, uint16_t *cmd_data)
{
	int irq;

	pthread_mutex_lock(&lline->status_mutex);
	lline->status |= MX_LSTATE_ATTACHED;
	pthread_mutex_unlock(&lline->status_mutex);
	irq = MX_IRQ_IDOLI;

	return irq;
}

// -----------------------------------------------------------------------
int mx_floppy_detach(struct mx_line *lline, uint16_t *cmd_data)
{ 
	pthread_mutex_lock(&lline->status_mutex);
	lline->status &= ~MX_LSTATE_ATTACHED;
	pthread_mutex_unlock(&lline->status_mutex);

	return MX_IRQ_IODLI;
}

// -----------------------------------------------------------------------
int mx_floppy_abort(struct mx_line *lline, uint16_t *cmd_data)
{ 
	return MX_IRQ_INABT;
}

// -----------------------------------------------------------------------
int mx_floppy_transmit(struct mx_line *lline, uint16_t *cmd_data)
{
	int irq;
	struct proto_floppy_data *proto_data = lline->proto_data;

	// check if there is a device connected
	if (!lline->dev || !lline->dev_data) {
		proto_data->ret_status = MX_FS_NO_DISC;
		irq = MX_IRQ_INTRA;
		goto fin;
	}

	LOG(L_FLOP, "Transmit operation %i: %s", proto_data->op, floppy_op_names[proto_data->op]);

	// TODO: temporary, so mega bootloader works
	proto_data->ret_status = MX_FS_NO_DISC;
	irq = MX_IRQ_ITRER;

fin:
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
		[MX_CMD_ATTACH] = { 3, 0, NULL, NULL, mx_floppy_attach },
		[MX_CMD_TRANSMIT] = { 4, 3, mx_floppy_transmit_decode, mx_floppy_transmit_encode, mx_floppy_transmit },
		[MX_CMD_DETACH] = { 0, 0, NULL, NULL, mx_floppy_detach },
		[MX_CMD_ABORT] = { 0, 0, NULL, NULL, mx_floppy_abort },
	}
};

// vim: tabstop=4 shiftwidth=4 autoindent
