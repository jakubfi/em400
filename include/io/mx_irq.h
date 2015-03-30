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

#ifndef MX_IRQ_H
#define MX_IRQ_H

#define MX_INTRQ_LEN 32
#define MX_INTSPEC_EMPTY 0x1FFFF

struct mx_irq {
	int line;
	int intr;
	struct mx_irq *next;
};

struct mx_irqq {
	struct mx_irq *head, *tail;
	int size;
	int maxlen;
	int chnum;
	uint32_t intspec; // accessed by CPU thred
};

enum mx_interrupts {
	MX_IRQ_INIEA = 0,	// interrupt no longer valid
	// special
	MX_IRQ_INSKA = 1,	// channel faulty
	MX_IRQ_IWYZE = 2,	// channel reset successfully
	MX_IRQ_IWYTE = 3,	// channel test finished
	// general
	MX_IRQ_INKON = 4,	// 'set configuration' rejected (configuration error, out of memory, configuration already set)
	MX_IRQ_IUKON = 5,	// 'set configuration' finished successfully
	MX_IRQ_INKOT = 6,	// 'set configuration' unsuccessfull (transmission errors)
	// line
	MX_IRQ_ISTRE = 7,	// 'report status' OK
	MX_IRQ_INSTR = 8,	// 'report status' rejected (previous 'report status' being executed)
	MX_IRQ_INKST = 9,	// 'report status' for non existent line
	MX_IRQ_IDOLI = 10,	// 'attach line' OK
	MX_IRQ_INDOL = 11,	// 'attach line' rejected (errors in field, line already attached, previous 'attach line' beind excecuted)
	MX_IRQ_INKDO = 12,	// 'attach line' for non existent line
	MX_IRQ_IETRA = 13,	// 'transmit' OK
	MX_IRQ_INTRA = 14,	// 'transmit' rejected (errors in field, line not attached, previous transmission ongoing)
	MX_IRQ_INKTR = 15,	// 'transmit' for non existent line
	MX_IRQ_ITRER = 16,	// 'transmit' finished with error (parity or other)
	MX_IRQ_ITRAB = 19,	// 'transmit' cancelled (as ordered by 'cancel transmission')
	MX_IRQ_IABTR = 20,	// 'cancel transmission' OK
	MX_IRQ_INABT = 21,	// 'cancel transmission' while no transmission
	MX_IRQ_INKAB = 22,	// 'cancel transmission' for nonexistent line
	MX_IRQ_IODLI = 23,	// 'detach line' OK
	MX_IRQ_INODL = 24,	// 'detach line' for a line with ongoing transmission
	MX_IRQ_INKOD = 25,	// 'detach line' for non existent line
	MX_IRQ_INPAO = 32,	// mera-multix transmission failure
	MX_IRQ_IPARE = 33,	// mera-multix parity error
	MX_IRQ_IOPRU = 34,	// operator request
	MX_IRQ_IEPS0 = 35,	// unknown control command, code=0
	MX_IRQ_IEPS6 = 36,	// unknown control command, code=6
	MX_IRQ_IEPS7 = 37,	// unknown control command, code=7
	MX_IRQ_IEPS8 = 38,	// unknown control command, code=8
	MX_IRQ_IEPSC = 39,	// unknown control command, code=C
	MX_IRQ_IEPSD = 40,	// unknown control command, code=D
	MX_IRQ_IEPSE = 41,	// unknown control command, code=E
	MX_IRQ_IEPSF = 42	// unknown control command, code=F
};

struct mx_irqq * mx_irqq_create(int chnum, int maxlen);
void mx_irqq_irq_delete(struct mx_irq *event);
void mx_irqq_clear(struct mx_irqq *queue);
int mx_irqq_size(struct mx_irqq *queue);
void mx_irqq_destroy(struct mx_irqq *queue);
int mx_irqq_enqueue(struct mx_irqq *queue, int intr, int line);
void mx_irqq_advance(struct mx_irqq *queue);
uint16_t mx_irqq_get_intspec(struct mx_irqq *queue);
void mx_irqq_cancel_current(struct mx_irqq *queue);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
