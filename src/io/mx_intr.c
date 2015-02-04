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

#include <stdlib.h>

#include "io/mx_intr.h"

#include "log.h"
#include "cpu/interrupts.h"

#define CHAN ((struct mx *)(chan))

const char *mx_intr_names[] = {
	"INIEA (intr no longer valid)",
	"INSKA (channel faulty)",
	"IWYZE (reset done)",
	"IWYTE (test done)",
	"INKON ('setconf' rejected)",
	"IUKON ('setconf' done)",
	"INKOT ('setconf' failed)",
	"ISTRE ('status' done)",
	"INSTR ('status' rejected)",
	"INKST ('status' no line)",
	"IDOLI ('attach' done)",
	"INDOL ('attach' rejected)",
	"INKDO ('attach' no line)",
	"IETRA ('transmit' done)",
	"INTRA ('transmit' rejected)",
	"INKTR ('transmit' no line)",
	"ITRER ('transmit' error)",
	"???",
	"???",
	"ITRAB ('transmit' cancelled)",
	"IABTR ('cancel' done)",
	"INABT ('cancel' while transmitting)",
	"INKAB ('cancel' no line)",
	"IODLI ('detach' done)",
	"INODL ('detach' while transmitting)",
	"INKOD ('detach' no line)",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
	"INPAO (mera-multix transmission failure)",
	"IPARE (parity error)",
	"IOPRU (OPRQ)",
	"IEPS0 (unknown command code=0)",
	"IEPS6 (unknown command code=6)",
	"IEPS7 (unknown command code=7)",
	"IEPS8 (unknown command code=8)",
	"IEPSC (unknown command code=C)",
	"IEPSD (unknown command code=D)",
	"IEPSE (unknown command code=E)",
	"IEPSF (unknown command code=F)"
};

// -----------------------------------------------------------------------
// call with intr_mutex locked
void mx_intr_report(struct mx *chan)
{
	if (CHAN->intr_head) {
		LOG(L_MX, 2, "MULTIX (ch:%i) report interrupt %i -> CPU", chan->num, chan->num + 12);
		int_set(chan->num + 12);
	}
}

// -----------------------------------------------------------------------
// call with intr_mutex locked
void mx_intr_clearq(struct mx *chan)
{
	struct mx_intr *inth = chan->intr_head;
	struct mx_intr *next;

	// clear interrupt queue
	while (inth) {
		next = inth->next;
		free(inth);
		inth = next;
	}
}

// -----------------------------------------------------------------------
// call with intr_mutex locked
void mx_intr_setq(struct mx *chan, struct mx_intr *mxintr)
{
	if (!mxintr) return;

	LOG(L_MX, 1, "MULTIX (ch:%i) set special intr %i: %s", chan->num, mxintr->intr, mx_intr_names[mxintr->intr]);

	mx_intr_clearq(chan);

	// set the interrupt and notify CPU
	chan->intr_tail = chan->intr_head = mxintr;
	chan->intrq_len = 1;
	mx_intr_report(chan);
}

// -----------------------------------------------------------------------
// call with intr_mutex locked
void mx_intr_enq(struct mx *chan, struct mx_intr *mxintr)
{
	if (!mxintr) return;

	LOG(L_MX, 1, "MULTIX (ch:%i lline:%i) enq line intr %i: %s (%i in queue)", chan->num, mxintr->llinen, mxintr->intr, mx_intr_names[mxintr->intr], chan->intrq_len);

	// 'channel faulty' if intr queue full or negative
	if ((chan->intrq_len >= MX_INTRQ_LEN) || (chan->intrq_len < 0)) {
		struct mx_intr *intfail = malloc(sizeof(struct mx_intr));
		intfail->llinen = 0;
		intfail->intr = MX_INTR_INSKA;
		intfail->next = NULL;
		mx_intr_setq(chan, intfail);
	// append interrupt to queue
	} else if (chan->intrq_len > 0) {
		chan->intr_tail->next = mxintr;
		chan->intr_tail = mxintr;
		chan->intrq_len++;
	// set queue and notify CPU
	} else {
		chan->intr_tail = chan->intr_head = mxintr;
		mx_intr_report(chan);
		chan->intrq_len = 1;
	}
}

// -----------------------------------------------------------------------
// call with intr_mutex locked
struct mx_intr * mx_intr_deq(struct mx *chan)
{
	struct mx_intr *inth = chan->intr_head;

	if (inth) {
		chan->intrq_len--;
		if (chan->intr_head == chan->intr_tail) {
			chan->intr_tail = chan->intr_head = NULL;
		} else {
			chan->intr_head = chan->intr_head->next;
		}
	}

	return inth;
}

// -----------------------------------------------------------------------
void mx_int(struct mx *chan, int llinen, int intr)
{
	struct mx_intr *mxintr = malloc(sizeof(struct mx_intr));
	if (!mxintr) {
		LOG(L_MX, 1, "MULTIX (ch:%i) cannot allocate memory for interrupt, interrupt LOST: spec:%i, lline: %i", chan->num, intr, llinen);
		return;
	}

	mxintr->llinen = llinen;
	mxintr->intr = intr;
	mxintr->next = NULL;

	pthread_mutex_lock(&chan->intr_mutex);

	// high priority interrupts
	if (intr <= MX_INTR_IWYTE) {
		mx_intr_setq(chan, mxintr);
	// line interrupts, queued
	} else {
		mx_intr_enq(chan, mxintr);
	}

	pthread_mutex_unlock(&CHAN->intr_mutex);
}

// vim: tabstop=4 shiftwidth=4 autoindent
