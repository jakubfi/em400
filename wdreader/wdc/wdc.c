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

#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>

#include "wctl.h"
#include "serial.h"

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(void)
{
	wdc_init();
	serial_init();

	const unsigned char buf_size = 5;
	char buf[buf_size+1];
	unsigned char rcount;
	unsigned char ret;
	int val;

	while (1) {
		rcount = serial_rx_string(buf, buf_size, '\r');
		buf[rcount] = '\0';

		switch (*buf) {
			case 's':
				ret = wdc_status();
				break;
			case 't':
				ret = wdc_track0();
				break;
			case 'i':
				ret = wdc_step_in();
				break;
			case 'o':
				ret = wdc_step_out();
				break;
			case 'c':
				val = atoi(buf+1);
				ret = wdc_seek(val);
				break;
			case 'h':
				val = atoi(buf+1);
				ret = wdc_head_sel(val);
				break;
			case 'd':
				val = atoi(buf+1);
				ret = wdc_drv_sel(val);
				break;
			default:
				ret = RET_ERR;
				break;
		}

		if (ret == RET_OK) {
			serial_tx_string("OK!\n\r");
		} else {
			serial_tx_string("ERR\n\r");
		}

	}

	return 0;
} 

// vim: tabstop=4
