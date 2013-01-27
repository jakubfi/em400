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
	wdc_init();
	serial_init();

	_delay_ms(1000);

	while (1) {
		char c = serial_rx_char();
		switch (c) {
			case 's':
				if (wdc_status() == RET_OK) {
					serial_tx_string("OK!");
				} else {
					serial_tx_string("ERR");
				}
				break;
			case 't':
				if (wdc_track0() == RET_OK) {
					serial_tx_string("OK!");
				} else {
					serial_tx_string("ERR");
				}
				break;
			case 'i':
				if (wdc_step_in() == RET_OK) {
					serial_tx_string("OK!");
				} else {
					serial_tx_string("ERR");
				}
				break;
			case 'o':
				if (wdc_step_out() == RET_OK) {
					serial_tx_string("OK!");
				} else {
					serial_tx_string("ERR");
				}
				break;
			case 'a':
				if (wdc_head_sel(0) == RET_OK) {
					serial_tx_string("OK!");
				} else {
					serial_tx_string("ERR");
				}
				break;
			case 'b':
				if (wdc_head_sel(1) == RET_OK) {
					serial_tx_string("OK!");
				} else {
					serial_tx_string("ERR");
				}
				break;
			case 'c':
				if (wdc_head_sel(2) == RET_OK) {
					serial_tx_string("OK!");
				} else {
					serial_tx_string("ERR");
				}
				break;
			case 'd':
				if (wdc_head_sel(3) == RET_OK) {
					serial_tx_string("OK!");
				} else {
					serial_tx_string("ERR");
				}
				break;
			default:
				serial_tx_string("ERR");
				break;
		}
	}
	return 0;
} 

// vim: tabstop=4
