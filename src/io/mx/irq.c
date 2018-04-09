//  Copyright (c) 2015-2018 Jakub Filipowicz <jakubf@gmail.com>
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

#include "io/mx/irq.h"

const char *mx_irq_names[] = {
	"INIEA: interrupt invalid",
	"INSKA: multix faulty",
	"IWYZE: reset OK",
	"IWYTE: test OK",
	"INKON: setconf rejected",
	"IUKON: setconf OK",
	"INKOT: setconf failed",
	"ISTRE: status OK",
	"INSTR: status rejected",
	"INKST: status failed, no such line",
	"IDOLI: attach OK",
	"INDOL: attach rejected",
	"INKDO: attach failed, no such line",
	"IETRA: transmit OK",
	"INTRA: transmit rejected",
	"INKTR: transmit failed, no such line",
	"ITRER: transmit failed",
	"IRQ17: unused",
	"IRQ18: unused",
	"ITRAB: transmit aborted",
	"IABTR: abort OK",
	"INABT: abort failed, no transmission",
	"INKAB: abort failed, no such line",
	"IODLI: detach OK",
	"INODL: detach failed, transmission active",
	"INKOD: detach failed, no such line",
	"IRQ26: unused",
	"IRQ27: unused",
	"IRQ28: unused",
	"IRQ29: unused",
	"IRQ30: unused",
	"IRQ31: unused",
	"INPAO: bus error",
	"IPARE: memory parity error",
	"IOPRU: OPRQ",
	"IEPS0: unknown command 0",
	"IEPS6: unknown command 6",
	"IEPS7: unknown command 7",
	"IEPS8: unknown command 8",
	"IEPSC: unknown command C",
	"IEPSD: unknown command D",
	"IEPSE: unknown command E",
	"IEPSF: unknown command F",
	"[invalid-IRQ-number]"
};

// -----------------------------------------------------------------------
const char * mx_irq_name(unsigned i)
{
	if (i < MX_IRQ_CNT) {
		return mx_irq_names[i];
	} else {
		return mx_irq_names[MX_IRQ_CNT];
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
