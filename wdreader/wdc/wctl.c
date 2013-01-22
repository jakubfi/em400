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

#include <avr/io.h>
#include <util/delay.h>

#include "wctl.h"

// ports where winchester signals are connected

#define STATUS_PORT	PORTA
#define STATUS_PIN	PINA
#define STATUS_DDR	DDRA

#define CTRL_PORT	PORTC
#define CTRL_PIN	PINC
#define CTRL_DDR	DDRC

// input: status signals pads

#define READY	0
#define SEEKOK	1
#define TRACK0	2

// output: drive selection pads

#define DRV1	0
#define DRV2	1
#define DRV3	2
#define DRV4	3

// output: head selection and movement pads

#define STEP		4
#define DIRIN		5
#define HEADSEL0	6
#define HEADSEL1	7

// status test macros

#define INIT_READY		~(STATUS_PIN & (_BV(READY) | _BV(SEEKOK) | _BV(TRACK0)))
#define INIT_NOT_READY	~INIT_READY
#define SEEK_COMPLETE	~(STATUS_PIN & _BV(SEEKOK))
#define DRIVE_READY		~(STATUS_PIN & _BV(READY))

// timings

#define DELAY_DIR			2
#define DELAY_STEP_PULSE	8
#define DELAY_STEP_PAUSE	28
#define DELAY_HEADSEL		8
#define DELAY_DRIVESEL		1

// disk geometry

#define CYL_MIN		0
#define CYL_MAX		614

unsigned char drive[4] = { DRV1, DRV2, DRV3, DRV4 };
unsigned int cylinder = 0;

// -----------------------------------------------------------------------
void _wdc_port_setup(void)
{
	// status port is an input port with pull-up
	SFIOR &= ~(_BV(PUD));
	STATUS_DDR = ~(_BV(READY) | _BV(SEEKOK) | _BV(TRACK0));
	STATUS_PORT = (_BV(READY) | _BV(SEEKOK) | _BV(TRACK0));

	// drv and head ports are output ports
	CTRL_DDR = _BV(DRV1) | _BV(DRV2) | _BV(DRV3) | _BV(DRV4) | _BV(HEADSEL0) | _BV(HEADSEL1) | _BV(STEP) | _BV(DIRIN);
	CTRL_PORT |= _BV(DRV1) | _BV(DRV2) | _BV(DRV3) | _BV(DRV4) | _BV(HEADSEL0) | _BV(HEADSEL1) | _BV(STEP) | _BV(DIRIN);
}

// -----------------------------------------------------------------------
int wdc_init(void)
{
	const unsigned char init_wait = 200;
	unsigned char wait_cycles = 100; // 20 seconds should be more than enough for the drive to spinup

	_wdc_port_setup();

	// select drive 1
	CTRL_PORT &= ~_BV(DRV1);

	// wait for spinup to complete
	while (INIT_NOT_READY && (wait_cycles > 0)) {
		_delay_ms(init_wait);
		wait_cycles--;
	}

	// drive initialized properly
	if (wait_cycles > 0) {
		return RET_OK;
	}

	return RET_ERR;
}

// -----------------------------------------------------------------------
int wdc_drv_sel(unsigned char drv)
{
	// clear all drive selection lines
	CTRL_PORT |= _BV(DRV1) | _BV(DRV2) | _BV(DRV3) | _BV(DRV4);

	if ((drv < 1) || (drv > 4)) {
		return RET_ERR;
	}

	// select drive specified by user
	CTRL_PORT &= ~_BV(drive[drv-1]);
	_delay_us(DELAY_DRIVESEL);

	while (~DRIVE_READY) {
		_delay_ms(1);
	}

	return RET_OK;
}

// -----------------------------------------------------------------------
void wdc_head_sel(unsigned char head)
{
	// clear head selection
	CTRL_PORT &= ~(_BV(HEADSEL0) | _BV(HEADSEL1));
	// select head
	CTRL_PORT |= ~head & (_BV(HEADSEL0) | _BV(HEADSEL1));
	_delay_us(DELAY_HEADSEL);
}

// -----------------------------------------------------------------------
inline void _wdc_dir_in(void)
{
	CTRL_PORT &= ~_BV(DIRIN);
	_delay_us(DELAY_DIR);
}

// -----------------------------------------------------------------------
inline void _wdc_dir_out(void)
{
	CTRL_PORT |= _BV(DIRIN);
	_delay_us(DELAY_DIR);
}

// -----------------------------------------------------------------------
inline void _wdc_step_pulse(void)
{
	CTRL_PORT &= ~_BV(STEP);
	_delay_us(DELAY_STEP_PULSE);
	CTRL_PORT |= _BV(STEP);
	_delay_us(DELAY_STEP_PAUSE);
}

// -----------------------------------------------------------------------
void _wdc_seek_reset(void)
{
	while (STATUS_PIN & _BV(TRACK0)) {
		_wdc_dir_out();
		_wdc_step_pulse();
		while (~SEEK_COMPLETE);
	}
	cylinder = 0;
}

// -----------------------------------------------------------------------
void wdc_seek(unsigned int cyl)
{
	if ((cyl < CYL_MIN) || (cyl > CYL_MAX) || (cyl == cylinder)) {
		// requested cylinder out of bounds or already at requested cylinder
		return;
	}

	int delta = cyl - cylinder;

	// set stepping direction
	if (delta > 0) {
		_wdc_dir_in();
		while (delta > 0) {
			_wdc_step_pulse();
			delta--;
		}
	} else if (delta < 0) {
		_wdc_dir_out();
		while (delta < 0) {
			_wdc_step_pulse();
			delta++;
		}
	} else {
	}

	cylinder = cyl;

	// wait until heads settle
	while (~SEEK_COMPLETE);
}

// -----------------------------------------------------------------------
void wdc_step_in(void)
{
	wdc_seek(cylinder+1);
}

// -----------------------------------------------------------------------
void wdc_step_out(void)
{
	wdc_seek(cylinder-1);
}

// vim: tabstop=4
