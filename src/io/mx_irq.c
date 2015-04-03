//  Copyright (c) 2015 Jakub Filipowicz <jakubf@gmail.com>
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

#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "atomic.h"
#include "cpu/interrupts.h"
#include "io/mx_irq.h"

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

static void mx_irqq_send(struct mx_irqq *queue);

// -----------------------------------------------------------------------
struct mx_irqq * mx_irqq_create(int chnum, int maxlen)
{
	struct mx_irqq *queue = malloc(sizeof(struct mx_irqq));
	if (!queue) {
		goto cleanup;
	}

	queue->chnum = chnum;
	queue->head = NULL;
	queue->tail = NULL;
	queue->size = 0;
	queue->maxlen = maxlen;
	atom_store(&queue->intspec, MX_INTSPEC_EMPTY);

	return queue;

cleanup:
	mx_irqq_destroy(queue);
	return NULL;
}

// -----------------------------------------------------------------------
void mx_irqq_irq_delete(struct mx_irq *irq)
{
	free(irq);
}

// -----------------------------------------------------------------------
void mx_irqq_clear(struct mx_irqq *queue)
{
	struct mx_irq *irq = queue->head;
	struct mx_irq *t;

	while (irq) {
		t = irq->next;
		mx_irqq_irq_delete(irq);
		irq = t;
	}

	queue->size = 0;
	queue->head = NULL;
	queue->tail = NULL;

	atom_store(&queue->intspec, MX_INTSPEC_EMPTY);
}

// -----------------------------------------------------------------------
int mx_irqq_size(struct mx_irqq *queue)
{
	return queue->size;
}

// -----------------------------------------------------------------------
void mx_irqq_destroy(struct mx_irqq *queue)
{
	if (!queue) return;

	mx_irqq_clear(queue);
	free(queue);
}

// -----------------------------------------------------------------------
int mx_irqq_enqueue(struct mx_irqq *queue, int intr, int line)
{
	LOG(L_MX, 3, "MULTIX (ch:%i) Enqueue interrupt %i (%s), line %i (queue size: %i)", queue->chnum, intr, mx_intr_names[intr], line, queue->size);

	// queue full?
	if ((queue->maxlen > 0) && (queue->size >= queue->maxlen)) {
		LOG(L_MX, 3, "MULTIX (ch:%i) Interrupt queue full", queue->chnum);
		return -1;
	}

	// create new irq
	struct mx_irq *irq = malloc(sizeof(struct mx_irq));
	if (!irq) {
		return -1;
	}

	irq->line = line;
	irq->intr = intr;
	irq->next = NULL;

	// append irq
	if (queue->tail) {
		queue->tail->next = irq;
		queue->tail = irq;
	} else {
		queue->tail = queue->head = irq;
	}

	queue->size++;

	// try sending IRQ to CPU
	mx_irqq_send(queue);

	return queue->size;
}

// -----------------------------------------------------------------------
void mx_irqq_advance(struct mx_irqq *queue)
{
	LOG(L_MX, 3, "MULTIX (ch:%i) advancing interrupt queue", queue->chnum);

	struct mx_irq *irq = queue->head;

	if (irq) {
		queue->head = irq->next;
		if (!queue->head) {
			queue->tail = NULL;
		}
		queue->size--;
	}

	mx_irqq_irq_delete(irq);
	mx_irqq_send(queue);
}

// -----------------------------------------------------------------------
static void mx_irqq_send(struct mx_irqq *queue)
{
	// interrupt queue empty
	if (!queue->head) {
		LOG(L_MX, 3, "MULTIX (ch:%i) No IRQ to send to CPU, queue is empty", queue->chnum);
		return;
	}

	// current interrupt has not been received by CPU
	if (atom_load(&queue->intspec) != MX_INTSPEC_EMPTY) {
		LOG(L_MX, 3, "MULTIX (ch:%i) Cannot send IRQ to CPU, previous one has not been received yet", queue->chnum);
		return;
	}

	struct mx_irq *irq = queue->head;

	LOG(L_MX, 3, "MULTIX (ch:%i) Sending interrupt to CPU", queue->chnum);

	// store interrupt specification
	atom_store(&queue->intspec, (irq->intr << 8) | irq->line);

	// notify CPU
	int_set(queue->chnum + 12);
}

// -----------------------------------------------------------------------
// called by CPU thread
uint16_t mx_irqq_get_intspec(struct mx_irqq *queue)
{
	uint16_t intspec = atom_load(&queue->intspec) & 0xFFFF;
	// intspec is most probably reset in H/W either:
	// * when CPU reads it
	// * or when interrupt routine does "MVI A,1  OUT KWINT"
	atom_store(&queue->intspec, MX_INTSPEC_EMPTY);
	LOG(L_MX, 3, "MULTIX (ch:%i) Sending intspec to CPU: 0x%04x (%s, line %i)", queue->chnum, intspec, mx_intr_names[intspec>>8], intspec & 0xFF);
	return intspec;
}

// -----------------------------------------------------------------------
// called by CPU thread
void mx_irqq_irq_requeue(struct mx_irqq *queue)
{
	atom_store(&queue->intspec, MX_INTSPEC_EMPTY);
	mx_irqq_send(queue);
}

// vim: tabstop=4 shiftwidth=4 autoindent
