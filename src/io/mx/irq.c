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
	"INIEA (interrupt no longer valid)",
	"INSKA (channel faulty)",
	"IWYZE ('reset' done)",
	"IWYTE ('test' done)",
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
	"IRQ17 (unused)",
	"IRQ18 (unused)",
	"ITRAB ('transmit' cancelled)",
	"IABTR ('cancel' done)",
	"INABT ('cancel' while transmitting)",
	"INKAB ('cancel' no line)",
	"IODLI ('detach' done)",
	"INODL ('detach' while not transmitting)",
	"INKOD ('detach' no line)",
	"IRQ26 (unused)",
	"IRQ27 (unused)",
	"IRQ28 (unused)",
	"IRQ29 (unused)",
	"IRQ30 (unused)",
	"IRQ31 (unused)",
	"INPAO (mera-multix transmission errors)",
	"IPARE (memory parity error)",
	"IOPRU (OPRQ)",
	"IEPS0 (unknown command code=0)",
	"IEPS6 (unknown command code=6)",
	"IEPS7 (unknown command code=7)",
	"IEPS8 (unknown command code=8)",
	"IEPSC (unknown command code=C)",
	"IEPSD (unknown command code=D)",
	"IEPSE (unknown command code=E)",
	"IEPSF (unknown command code=F)",
	"(invalid IRQ number)"
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
