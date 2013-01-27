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

#define STATUS_PORT	PORTC
#define STATUS_PIN	PINC
#define STATUS_DDR	DDRC

#define CTRL_PORT	PORTA
#define CTRL_PIN	PINA
#define CTRL_DDR	DDRA

// input: status signals pads

#define SEEKOK	0
#define TRACK0	1
#define INDEX	2
#define READY	3

// output: drive selection pads

#define DRV1	4
#define DRV2	3
#define DRV3	2
#define DRV4	1

// output: head selection and movement pads

#define STEP		5
#define DIRIN		0
#define HEADSEL0	7
#define HEADSEL1	6

// status test macros

#define SEEK_COMPLETE	!(STATUS_PIN & _BV(SEEKOK))
#define DRIVE_READY		!(STATUS_PIN & (_BV(READY) | _BV(SEEKOK)))
#define TRACK_IS_0		!(STATUS_PIN & _BV(TRACK0))

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
	// drv and head ports are output ports
	CTRL_PORT |= _BV(DRV1) | _BV(DRV2) | _BV(DRV3) | _BV(DRV4) | _BV(HEADSEL0) | _BV(HEADSEL1) | _BV(STEP) | _BV(DIRIN);
	CTRL_DDR = _BV(DRV1) | _BV(DRV2) | _BV(DRV3) | _BV(DRV4) | _BV(HEADSEL0) | _BV(HEADSEL1) | _BV(STEP) | _BV(DIRIN);

	// status port is an input port (with pull-up)
	SFIOR &= ~(_BV(PUD));
	STATUS_DDR = ~(_BV(READY) | _BV(SEEKOK) | _BV(TRACK0) | _BV(INDEX));
	STATUS_PORT = (_BV(READY) | _BV(SEEKOK) | _BV(TRACK0) | _BV(INDEX));
}

// -----------------------------------------------------------------------
void wdc_init(void)
{
	_wdc_port_setup();

	// select drive 1
	CTRL_PORT &= ~_BV(DRV1);
}

// -----------------------------------------------------------------------
int wdc_status(void)
{
	if (DRIVE_READY) {
		return RET_OK;
	} else {
		return RET_ERR;
	}
}

// -----------------------------------------------------------------------
int wdc_track0(void)
{
	if (TRACK_IS_0) {
		return RET_OK;
	} else {
		return RET_ERR;
	}
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

	while (!DRIVE_READY) {
		_delay_ms(1);
	}

	return RET_OK;
}

// -----------------------------------------------------------------------
int wdc_head_sel(unsigned char head)
{
	switch (head) {
		case 0:
			CTRL_PORT |= _BV(HEADSEL0) | _BV(HEADSEL1);
			break;
		case 1:
			CTRL_PORT |= _BV(HEADSEL1);
			CTRL_PORT &= ~_BV(HEADSEL0);
			break;
		case 2:
			CTRL_PORT |= _BV(HEADSEL0);
			CTRL_PORT &= ~_BV(HEADSEL1);
			break;
		case 3:
			CTRL_PORT &= ~(_BV(HEADSEL0) | _BV(HEADSEL1));
			break;
		default:
			return RET_ERR;
	}
	_delay_us(DELAY_HEADSEL);
	return RET_OK;
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
int wdc_seek(unsigned int cyl)
{
    const unsigned char seek_wait = 1;
    unsigned char wait_cycles = 250; // 250ms timeout for seek

	if ((cyl < CYL_MIN) || (cyl > CYL_MAX) || (cyl == cylinder)) {
		// requested cylinder out of bounds or already at requested cylinder
		return RET_ERR;
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
	while (!SEEK_COMPLETE && (wait_cycles > 0)) {
		_delay_ms(seek_wait);
		wait_cycles--;
	}

    // seek completed properly
    if (wait_cycles > 0) {
        return RET_OK;
    }

	return RET_ERR;
}

// -----------------------------------------------------------------------
int wdc_step_in(void)
{
	return wdc_seek(cylinder+1);
}

// -----------------------------------------------------------------------
int wdc_step_out(void)
{
	return wdc_seek(cylinder-1);
}

// vim: tabstop=4
