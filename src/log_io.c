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

#include <strings.h>
#include <stdlib.h>

#include "io/cmem_m9425.h"

#include "utils.h"

/*
// -----------------------------------------------------------------------
int decode_mxpsuk_pl(struct mx_cf_sc_pl *pl, char *b)
{
	int pos = 0;
	if (pl->used) {
		pos += sprintf(b+pos, "  Direction: (%i) ", pl->dir);
		switch (pl->dir) {
			case MX_DIR_OUTPUT:
				pos += sprintf(b+pos, "output");
				break;
			case MX_DIR_INPUT:
				pos += sprintf(b+pos, "input");
				break;
			case MX_DIR_HALF_DUPLEX:
				pos += sprintf(b+pos, "half-duplex");
				break;
			case MX_DIR_FULL_DUPLEX:
				pos += sprintf(b+pos, "full-duplex");
				break;
			default:
				pos += sprintf(b+pos, "unknown");
				break;
		}
		pos += sprintf(b+pos, "\n");

		pos += sprintf(b+pos, "  Line type: (%i) ", pl->type);
		switch (pl->type) {
			case MX_PHY_USART_ASYNC:
				pos += sprintf(b+pos, "USART asynch");
				break;
			case MX_PHY_8255:
				pos += sprintf(b+pos, "8255 (parallel)");
				break;
			case MX_PHY_USART_SYNC:
				pos += sprintf(b+pos, "USART synch");
				break;
			case MX_PHY_WINCHESTER:
				pos += sprintf(b+pos, "winchester");
				break;
			case MX_PHY_MTAPE:
				pos += sprintf(b+pos, "magnetic tape");
				break;
			case MX_PHY_FLOPPY:
				pos += sprintf(b+pos, "floppy disk");
				break;
			default:
				pos += sprintf(b+pos, "unknown");
				break;
		}
		pos += sprintf(b+pos, "\n");

	} else {
		pos += sprintf(b+pos, "  not used\n");
	}
	return pos;
}

// -----------------------------------------------------------------------
int decode_mxpsuk_ll_winch(struct mx_ll_winch *winch, char *b)
{
	int pos = 0;
	pos += sprintf(b+pos, "    Winchester type: (%i) ", winch->type);
	switch (winch->type) {
		case MX_WINCH_BASF:
			pos += sprintf(b+pos, "BASF");
			break;
		case MX_WINCH_NEC:
			pos += sprintf(b+pos, "NEC");
			break;
		default:
			pos += sprintf(b+pos, "unknown");
			break;
	}
	pos += sprintf(b+pos, "\n");
	pos += sprintf(b+pos, "    Formatting: %s\n", winch->format_protect ? "disallowed" : "allowed");
	return pos;
}

// -----------------------------------------------------------------------
int decode_mxpsuk_ll_floppy(struct mx_ll_floppy *floppy, char *b)
{
	int pos = 0;
	pos += sprintf(b+pos, "    Floppy type: (%i) ", floppy->type);
	switch (floppy->type) {
		case 0:
			pos += sprintf(b+pos, "40 cyl. (SD)");
			break;
		case 1:
			pos += sprintf(b+pos, "80 cyl. (DD)");
			break;
		case 2:
			pos += sprintf(b+pos, "80 cyl. (DD/HD)");
			break;
		default:
			pos += sprintf(b+pos, "unknown");
			break;
	}
	pos += sprintf(b+pos, "\n");
	pos += sprintf(b+pos, "    Formatting: %s\n", floppy->format_protect ? "disallowed" : "allowed");
	return pos;

}

// -----------------------------------------------------------------------
int decode_mxpsuk_ll(struct mx_cf_sc_ll *ll, char *b)
{
	int pos = 0;
	pos += sprintf(b+pos, "  Physical line id: %i\n", ll->pl_id);
	pos += sprintf(b+pos, "  Protocol: (%i) ", ll->proto);
	switch (ll->proto) {
		case MX_PROTO_PUNCH_READER:
			pos += sprintf(b+pos, "punched tape reader\n");
			break;
		case MX_PROTO_PUNCHER:
			pos += sprintf(b+pos, "tape puncher\n");
			break;
		case MX_PROTO_TERMINAL:
			pos += sprintf(b+pos, "terminal\n");
			break;
		case MX_PROTO_SOM_PUNCH_READER:
			pos += sprintf(b+pos, "SOM punched tape reader\n");
			break;
		case MX_PROTO_SOM_PUNCHER:
			pos += sprintf(b+pos, "SOM tape puncher\n");
			break;
		case MX_PROTO_SOM_TERMINAL:
			pos += sprintf(b+pos, "SOM terminal\n");
			break;
		case MX_PROTO_WINCHESTER:
			pos += sprintf(b+pos, "winchester\n");
			if (ll->winch) {
				pos += decode_mxpsuk_ll_winch(ll->winch, b+pos);
			} else {
				pos += sprintf(b+pos, "Missing Winchester logical line description\n");
			}
			break;
		case MX_PROTO_MTAPE:
			pos += sprintf(b+pos, "magnetic tape\n");
			pos += sprintf(b+pos, "  Formatter: %i\n", ll->formatter);
			break;
		case MX_PROTO_FLOPPY:
			pos += sprintf(b+pos, "floppy disk\n");
			if (ll->floppy) {
				pos += decode_mxpsuk_ll_floppy(ll->floppy, b+pos);
			} else {
				pos += sprintf(b+pos, "Missing floppy logical line description\n");
			}
			break;
		default:
			pos += sprintf(b+pos, "unknown\n");
			break;
	}
	return pos;
}

// -----------------------------------------------------------------------
char * decode_mxpsuk(int nb, uint16_t addr, int arg)
{
	char *buf = malloc(16*1024);
	char *b = buf;
	int pos = 0;
	int i;

	if (!buf) {
		return NULL;
	}
	struct mx_cf_sc *uk = calloc(1, sizeof(struct mx_cf_sc));
	if (!uk) {
		pos += sprintf(b+pos, "Cannot allocate memory");
		return buf;
	}

	int res = mx_decode_cf_sc(addr, uk);

	if (res != E_OK) {
		pos += sprintf(b+pos, "ERROR DECODING CONFIGURATION FIELD, RESULTS ARE PROBABLY WRONG!\n");
		pos += sprintf(b+pos, "Error %i for line %i: ", uk->err_code, uk->err_line);
		switch (uk->err_code) {
			case MX_SC_E_CONFSET:			pos += sprintf(b+pos, "configuration already set\n"); break;
			case MX_SC_E_NUMLINES:			pos += sprintf(b+pos, "wrong number of physical or logical lines\n"); break;
			case MX_SC_E_DEVTYPE:			pos += sprintf(b+pos, "unknown device type in physical line description\n"); break;
			case MX_SC_E_DIR:				pos += sprintf(b+pos, "unknown transmission direction\n"); break;
			case MX_SC_E_PHY_INCOMPLETE:	pos += sprintf(b+pos, "incomplete physical line description\n"); break;
			case MX_SC_E_PROTO_MISSING:		pos += sprintf(b+pos, "missing protocol\n"); break;
			case MX_SC_E_PHY_UNUSED:		pos += sprintf(b+pos, "physical line is not used\n"); break;
			case MX_SC_E_DIR_MISMATCH:		pos += sprintf(b+pos, "device vs. protocol transmission dricetion mismatch\n"); break;
			case MX_SC_E_PHY_BUSY:			pos += sprintf(b+pos, "physical line is busy\n"); break;
			case MX_SC_E_NOMEM:				pos += sprintf(b+pos, "memory exhausted\n"); break;
			case MX_SC_E_PROTO_MISMATCH:	pos += sprintf(b+pos, "protocol vs. physical line type mismatch\n"); break;
			case MX_SC_E_PROTO_PARAMS:		pos += sprintf(b+pos, "wrong protocol parameters\n"); break;
			default:						pos += sprintf(b+pos, "unknown\b"); break;
		}
	} else {
		pos += sprintf(b+pos, "Field decoded correctly\n");
	}

	pos += sprintf(b+pos, "Line descriptions: physical = %i, logical = %i\n", uk->pl_desc_count, uk->ll_desc_count);
	pos += sprintf(b+pos, "-------------------------------------\n");

	if (!uk->pl) {
		pos += sprintf(b+pos, "Missing physical lines configuration.");
		return buf;
	}

	if (!uk->pl) {
		pos += sprintf(b+pos, "Missing logical lines configuration.");
		return buf;
	}

	int start_ll = 0;
	for (i=0 ; i<uk->pl_desc_count ; i++) {
		pos += sprintf(b+pos, "PHYSICAL LINES %i - %i (count: %i):\n", start_ll, start_ll+uk->pl[i].count-1, uk->pl[i].count);
		pos += decode_mxpsuk_pl((uk->pl)+i, b+pos);
		start_ll += uk->pl[i].count;
	}
	pos += sprintf(b+pos, "-------------------------------------\n");
	for (i=0 ; i<uk->ll_desc_count ; i++) {
		pos += sprintf(b+pos, "LOGICAL LINE %i\n", i);
		pos += decode_mxpsuk_ll((uk->ll)+i, b+pos);
	}

	mx_free_cf_sc(uk);

	return buf;
}

// -----------------------------------------------------------------------
char * decode_mxpsdl(int nb, uint16_t addr, int arg)
{
	return NULL;
}

// -----------------------------------------------------------------------
int decode_mxpst_transmit_winch(struct mx_winch_cf_transmit *t, char *b)
{
	int pos = 0;

	pos += sprintf(b+pos, "Starting logical sector: %i\n", t->sector);
	pos += sprintf(b+pos, "Destination: CPU=%i, NB=%i, ADDR=0x%04x, length=%i\n", t->cpu, t->nb, t->addr, t->len);
	pos += sprintf(b+pos, "Ignore CRC=%s, Fill last sector=%s, Watch EOF=%s\n", t->ign_crc?"y":"n", t->sector_fill?"y":"n", t->watch_eof?"y":"n");

	return pos;
}

// -----------------------------------------------------------------------
char * decode_mxpst_winch(int nb, uint16_t addr, int arg)
{
	char *buf = malloc(16*1024);
	char *b = buf;
	int pos = 0;

	if (!buf) {
		return NULL;
	}

	struct mx_winch_cf_t *t = mx_winch_cf_t_decode(addr);
	if (!t) {
		pos += sprintf(b+pos, "Error decoding field");
		return buf;
	}

	pos += sprintf(b+pos, "Operation: (%i) ", t->oper);
	switch (t->oper) {
		case MX_WINCH_FORMAT_SPARE:
			pos += sprintf(b+pos, "Format spare area\n(no additional arguments)\n)");
			break;
		case MX_WINCH_FORMAT:
			pos += sprintf(b+pos, "Format track (optionally move sectors to spare area)\n");
			char *map = int2binf("................", t->format->sector_map, 16);
			pos += sprintf(b+pos, "Sector relocation map: %s\n", map);
			pos += sprintf(b+pos, "Starting sector: %i\n", t->format->start_sector);
			free(map);
			break;
		case MX_WINCH_READ:
			pos += sprintf(b+pos, "Read\n");
			pos += decode_mxpst_transmit_winch(t->transmit, buf+pos);
			break;
		case MX_WINCH_WRITE:
			pos += sprintf(b+pos, "Write\n");
			pos += decode_mxpst_transmit_winch(t->transmit, buf+pos);
			break;
		case MX_WINCH_PARK:
			pos += sprintf(b+pos, "Park\n");
			pos += sprintf(b+pos, "Cylinder: %i\n", t->park->cylinder);
			break;
		default:
			pos += sprintf(b+pos, "unknown\n");
			break;
	}

	mx_winch_cf_t_free(t);
	return buf;
}

// -----------------------------------------------------------------------
char * decode_mxpst_term(int nb, uint16_t addr, int arg)
{
	char *buf = malloc(16*1024);
	char *b = buf;
	int pos = 0;

	if (!buf) {
		return NULL;
	}

	struct mx_term_cf_transmit_t *t = mx_term_cf_transmit_decode(addr);
	if (!t) {
		pos += sprintf(b+pos, "Error decoding field");
		return buf;
	}

	pos += sprintf(b+pos, "Send (%s%s%s): Len: %i, buf addr: %i : 0x%04x, eot char: #%i, start byte pos: %i\n",
		(t->opts & MX_TERM_TX_SEND_BY_SIZE) ? "by_size " : "x ",
		(t->opts & MX_TERM_TX_SEND_BY_EOT_EXCL) ? "by_eot- " : "x ",
		(t->opts & MX_TERM_TX_SEND_BY_EOT_INCL) ? "by_eot+" : "x",
		t->send_len,
		t->send_nb,
		t->send_buf_addr,
		t->send_eot_char,
		t->send_start_byte);

	pos += sprintf(b+pos, "Recv (%s%s%s): Len: %i, buf addr: %i : 0x%04x, eot char: #%i, eot char2: #%i, start byte pos: %i\n",
		(t->opts & MX_TERM_TX_RECV_BY_SIZE) ? "by_size " : "x ",
		(t->opts & MX_TERM_TX_RECV_BY_EOT_EXCL) ? "by_eot- " : "x ",
		(t->opts & MX_TERM_TX_RECV_BY_EOT_INCL) ? "by_eot+" : "x",
		t->recv_len,
		t->recv_nb,
		t->recv_buf_addr,
		t->recv_eot_char,
		t->recv_eot_char2,
		t->send_start_byte);

	pos += sprintf(b+pos, "Timeout: %i, Prompt: \"%s\", Flags: %s%s\n",
		t->timeout,
		t->prompt_text,
		(t->opts & MX_TERM_TX_ECHO) ? "echo " : "",
		(t->opts & MX_TERM_TX_PROMPT) ? "prompt " : "");

	mx_term_cf_transmit_free(t);
	return buf;
}

// -----------------------------------------------------------------------
char * decode_cmempst(int nb, uint16_t addr, int arg)
{
	char *buf = malloc(16*1024);
	char *b = buf;
	int pos = 0;

	if (!buf) {
		return NULL;
	}

	struct cmem_m9425_cf_t *t = calloc(1, sizeof(struct cmem_m9425_cf_t));

	if (!t) {
		free(buf);
		return NULL;
	}

	int ret = cmem_m9425_decode_cf_t(addr, t);
	if (ret != E_OK) {
		pos += sprintf(b+pos, "Error decoding: %s", get_error(ret));
		return buf;
	}

	pos += sprintf(b+pos, "CF length: %i\n", t->cf_len);

	pos += sprintf(b+pos, "Operation: ");
	if (t->oper & 2) {
		pos += sprintf(b+pos, "write");
	} else {
		pos += sprintf(b+pos, "read");
	}
	if (t->oper & 1) {
		pos += sprintf(b+pos, " address");
	} else {
		pos += sprintf(b+pos, " data");
	}
	pos += sprintf(b+pos, "\n");
	pos += sprintf(b+pos, "PLATTER: %i, HEAD: %i, CYL: %i, SECTOR: %i\n", t->platter, t->head, t->cyl, t->sector);
	pos += sprintf(b+pos, "Destination: CPU=%i, NB=%i, ADDR=%i, length=%i\n", t->cpu, t->nb, t->addr, t->len);
	pos += sprintf(b+pos, "Ignore: wprotect=%s, defects=%s, key=%s, eof=%s\n", t->ign_wrprotect?"y":"n", t->ign_defects?"y":"n", t->ign_key?"y":"n", t->ign_eof?"y":"n");
	pos += sprintf(b+pos, "Key: %i\n", t->key);

	free(t);

	return buf;
}
*/
// vim: tabstop=4 shiftwidth=4 autoindent
