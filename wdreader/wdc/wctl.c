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

#define DRV_MAX		4
#define CYL_MIN		0
#define CYL_MAX		614

unsigned char drive_map[DRV_MAX+1] = { 0, _BV(DRV1), _BV(DRV2), _BV(DRV3), _BV(DRV4) };
unsigned char drv_selected = 0;
unsigned int cylinder[DRV_MAX+1] = { 0 };

// -----------------------------------------------------------------------
void wdc_init(void)
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
unsigned char wdc_status(void)
{
	if (DRIVE_READY) {
		return RET_OK;
	} else {
		return RET_ERR;
	}
}

// -----------------------------------------------------------------------
unsigned char wdc_track0(void)
{
	if (TRACK_IS_0) {
		return RET_OK;
	} else {
		return RET_ERR;
	}
}

// -----------------------------------------------------------------------
unsigned char wdc_drv_sel(unsigned char drv)
{
	// clear all drive selection lines anyway (even if drive is bogus)
	CTRL_PORT |= _BV(DRV1) | _BV(DRV2) | _BV(DRV3) | _BV(DRV4);
	drv_selected = 0;

	// 250ms timeout for drive select
	const unsigned char select_wait = 1;
	unsigned char wait_cycles = 250;

	// no such drive
	if ((drv < 0) || (drv > DRV_MAX)) {
		return RET_ERR;
	}

	// 'none' drive
	if (drv == 0) {
		_delay_us(DELAY_DRIVESEL);
		return RET_OK;
	}

	// real drive - select the drive
	drv_selected = drv;
	CTRL_PORT &= ~drive_map[drv];
	_delay_us(DELAY_DRIVESEL);

	// wait for the drive to report 'ready'
	while (!DRIVE_READY && (wait_cycles > 0)) {
		_delay_ms(select_wait);
		wait_cycles--;
	}

	// timeout occured
	if (wait_cycles <= 0) {
		return RET_ERR;
	}

	return RET_OK;
}

// -----------------------------------------------------------------------
unsigned char wdc_head_sel(unsigned char head)
{
	if (drv_selected == 0) return RET_ERR;

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
unsigned char wdc_seek(unsigned int cyl)
{
    const unsigned char seek_wait = 1;
    unsigned char wait_cycles = 250; // 250ms timeout for seek

	if (drv_selected == 0) return RET_ERR;

	// requested cylinder out of bounds
	if (cyl < CYL_MIN) return RET_ERR;
	// commented out to allow heads parking
	// if (cyl > CYL_MAX) return RET_ERR;

	int delta = cyl - cylinder[drv_selected];

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
		return RET_OK;
	}

	cylinder[drv_selected] = cyl;

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
unsigned char wdc_step_in(void)
{
	return wdc_seek(cylinder[drv_selected]+1);
}

// -----------------------------------------------------------------------
unsigned char wdc_step_out(void)
{
	return wdc_seek(cylinder[drv_selected]-1);
}

// vim: tabstop=4
