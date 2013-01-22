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

// atmega162, 8MHz internal RC oscillator

#include <avr/io.h>
#include <util/delay.h>

#include "wctl.h"
#include "serial.h"

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(void)
{
	serial_init();

	serial_tx_string("Initializing Winchester...\n\r");
	if (wdc_init() == RET_ERR) {
		serial_tx_string("FAILED.\n\r");
		return 1;
	}

	serial_tx_string("READY.\n\r");

	while (1) {
		char c = serial_rx_char();
		serial_tx_char(c);
		switch (c) {
			case 'i':
				wdc_step_in();
				break;
			case 'o':
				wdc_step_out();
				break;
			case '0':
				wdc_seek(0);
				break;
			case '1':
				wdc_seek(614);
				break;
		}
	}
	return 0;
} 

// vim: tabstop=4
