//  Copyright (c) 2013-2014 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef MX_INTR_H
#define MX_INTR_H

#include "io/io.h"
#include "io/mx.h"

#define MX_INTRQ_LEN 32

extern const char *mx_intr_names[];

struct mx_chan;

// -----------------------------------------------------------------------
struct mx_intr {
	unsigned llinen;
	unsigned intr;
	struct mx_intr *next;
};

enum mx_intrs {
	MX_INTR_INIEA = 0,	// interrupt no longer valid
	// special
	MX_INTR_INSKA = 1,	// channel faulty
	MX_INTR_IWYZE = 2,	// channel reset successfully
	MX_INTR_IWYTE = 3,	// channel test finished
	// general
	MX_INTR_INKON = 4,	// 'set configuration' rejected (configuration error, out of memory, configuration already set)
	MX_INTR_IUKON = 5,	// 'set configuration' finished successfully
	MX_INTR_INKOT = 6,	// 'set configuration' unsuccessfull (transmission errors)
	// line
	MX_INTR_ISTRE = 7,	// 'report status' OK
	MX_INTR_INSTR = 8,	// 'report status' rejected (previous 'report status' being executed)
	MX_INTR_INKST = 9,	// 'report status' for non existent line
	MX_INTR_IDOLI = 10,	// 'attach line' OK
	MX_INTR_INDOL = 11,	// 'attach line' rejected (errors in field, line already attached, previous 'attach line' beind excecuted)
	MX_INTR_INKDO = 12,	// 'attach line' for non existent line
	MX_INTR_IETRA = 13,	// 'transmit' OK
	MX_INTR_INTRA = 14,	// 'transmit' rejected (errors in field, line not attached, previous transmission ongoing)
	MX_INTR_INKTR = 15,	// 'transmit' for non existent line
	MX_INTR_ITRER = 16,	// 'transmit' finished with error (parity or other)
	MX_INTR_ITRAB = 19,	// 'transmit' cancelled (as ordered by 'cancel transmission')
	MX_INTR_IABTR = 20,	// 'cancel transmission' OK
	MX_INTR_INABT = 21,	// 'cancel transmission' while no transmission
	MX_INTR_INKAB = 22,	// 'cancel transmission' for nonexistent line
	MX_INTR_IODLI = 23,	// 'detach line' OK
	MX_INTR_INODL = 24,	// 'detach line' for a line with ongoing transmission
	MX_INTR_INKOD = 25,	// 'detach line' for non existent line
	MX_INTR_INPAO = 32,	// mera-multix transmission failure
	MX_INTR_IPARE = 33,	// mera-multix parity error
	MX_INTR_IOPRU = 34,	// operator request
	MX_INTR_IEPS0 = 35,	// unknown control command, code=0
	MX_INTR_IEPS6 = 36,	// unknown control command, code=6
	MX_INTR_IEPS7 = 37,	// unknown control command, code=7
	MX_INTR_IEPS8 = 38,	// unknown control command, code=8
	MX_INTR_IEPSC = 39,	// unknown control command, code=C
	MX_INTR_IEPSD = 40,	// unknown control command, code=D
	MX_INTR_IEPSE = 41,	// unknown control command, code=E
	MX_INTR_IEPSF = 42	// unknown control command, code=F
};

void mx_intr_report(struct mx_chan *chan);
void mx_intr_clearq(struct mx_chan *chan);
void mx_intr_setq(struct mx_chan *chan, struct mx_intr *mxintr);
void mx_intr_enq(struct mx_chan *chan, struct mx_intr *mxintr);
struct mx_intr * mx_intr_deq(struct mx_chan *chan);
void mx_int(struct mx_chan *chan, int llinen, int intr);


#endif

// vim: tabstop=4 shiftwidth=4 autoindent
