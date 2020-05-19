//  Copyright (c) 2017 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef __FPGA_H
#define __FPGA_H

#include <inttypes.h>
#include "cfg.h"

struct iob_msg {
	int is_valid;
	char *invalid_reason;
	uint8_t cmd;
	int is_req;
	int b_argc;
	int has_a1;
	int has_a2;
	int has_a3;
	int pn;
	int qb;
	int nb;
	uint16_t ad;
	uint16_t dt;
	int io_dir;
};

struct iob_cp_status {
	uint16_t data;
	int rot;
	int leds;
};

enum iob_status_leds {
	IOB_LED_ALARM	= 1 << 0,
	IOB_LED_WAIT	= 1 << 1,
	IOB_LED_RUN		= 1 << 2,
	IOB_LED_IRQ		= 1 << 3,
	IOB_LED_MC		= 1 << 4,
	IOB_LED_P		= 1 << 5,
	IOB_LED_Q		= 1 << 6,
	IOB_LED_CLOCK	= 1 << 7,
	IOB_LED_STOPN	= 1 << 8,
	IOB_LED_MODE	= 1 << 9
};

enum iob_requests {
	IOB_CMD_PA	= 0b0000,	// ->FPGA  power alarm
	IOB_CMD_CL	= 0b0001,	// <-FPGA  clear
	IOB_CMD_W	= 0b0010,	// ->FPGA  write
	IOB_CMD_R	= 0b0011,	// ->FPGA  read
	IOB_CMD_S	= 0b0100,	// <-FPGA  send
	IOB_CMD_F	= 0b0101,	// <-FPGA  fetch
	IOB_CMD_IN	= 0b0110,	// ->FPGA  interrupt
	IOB_CMD_CPD	= 0b1000,	// ->FPGA  CP: set data keys
	IOB_CMD_CPR	= 0b1001,	// ->FPGA  CP: set rotary switch
	IOB_CMD_CPF	= 0b1010,	// ->FPGA  CP: set function key
	IOB_CMD_CPS	= 0b1011	// ->FPGA  CP: get status
};

enum iob_responses {
	IOB_CMD_EN = 0b0001,	// ->FPGA
	IOB_CMD_OK = 0b0010,	// <>FPGA
	IOB_CMD_PE = 0b0011,	// <>FPGA
	IOB_CMD_NO = 0b0100		// <-FPGA
};

enum iob_fnkeys {
	IOB_FN_START	= 0b0000,
	IOB_FN_MODE		= 0b0001,
	IOB_FN_CLOCK	= 0b0010,
	IOB_FN_STOPN	= 0b0011,
	IOB_FN_STEP		= 0b0100,
	IOB_FN_FETCH	= 0b0101,
	IOB_FN_STORE	= 0b0110,
	IOB_FN_CYCLE	= 0b0111,
	IOB_FN_LOAD		= 0b1000,
	IOB_FN_BIN		= 0b1001,
	IOB_FN_OPRQ		= 0b1010,
	IOB_FN_CLEAR	= 0b1011
};

int iob_init(em400_cfg *cfg);
void iob_close();
void iob_quit();
void iob_loop();

void iob_reply_send(int bus, struct iob_msg *mi, int io_res);
void iob_int_send(int x);
void iob_pa_send();
int iob_mem_get(int nb, uint16_t addr, uint16_t *data);
int iob_mem_put(int nb, uint16_t addr, uint16_t data);
int iob_mem_mget(int nb, uint16_t saddr, uint16_t *dest, int count);
int iob_mem_mput(int nb, uint16_t saddr, uint16_t *src, int count);

void iob_cp_set_keys(uint16_t k);
void iob_cp_set_rotary(int r);
void iob_cp_set_fn(int fn, int v);
struct iob_cp_status * iob_cp_get_status();

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
