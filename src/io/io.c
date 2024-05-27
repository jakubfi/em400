//  Copyright (c) 2012-2013 Jakub Filipowicz <jakubf@gmail.com>
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

#include <inttypes.h>
#include <stdlib.h>

#include "mem/mem.h"
#include "cpu/cpu.h"
#include "cpu/interrupts.h"
#include "io/chan.h"
#include "fpga/iobus.h"

#include "cfg.h"
#include "utils/utils.h"
#include "log.h"

/*

  I/O stack looks something like this:

  .---------------------------------------------------------------------.
  |                                 CPU                                 |---+
  `---------------------------------------------------------------------'   |
  .---------------------------------------------------------------------.   |
  |                                 I/O                                 |   |
  `---------------------------------------------------------------------'    > CPU thread
  .---------------------------------------------------------------------.   |
  |                                 CHAN                                |   |
  `---------------------------------------------------------------------'   |
  .---------------------------------. .-------------------------. .-----.   |
  |             MULTIX              | |           CCHAR         | | ... |---+ chan_cmd()
  +- - - - - - - - - - - - - - - - -+ +- - - - - - - - - - - - -+ +- - -+   |
  |        device protocols         | |     device protocols    | | ... |   |
  `---------------------------------' `-------------------------' `-----'   |
  .---------------------------------------------------------------------.    > possible channel thread(s)
  |                                 DEV                                 |   |
  `---------------------------------------------------------------------'   |
  .------------. .-------------. .------------. .-----------. .---------.   |
  |  dev_term  | |  dev_winch  | |  dev_9425  | | dev_flop  | | dev_... |---+ dev_read(), dev_write(), dev_ctl()
  `------------' `-------------' `------------' `-----------' `---------'   |
  .------------. .-------------. .------------. .-----------. .---------.    > optional device thread(s)
  |  terminal  | |  e4image    | |  e4image   | |  e4image  | |   ...   |   |
  `------------' `-------------' `------------' `-----------' `---------'   |
*/

static struct chan *io_chan[IO_MAX_CHAN];
static const char *io_result_names[] = { "NO DEVICE", "ENGAGED", "OK", "PARITY ERROR" };
static int fpga;

// -----------------------------------------------------------------------
int io_init(em400_cfg *cfg)
{
	fpga = cfg_getbool(cfg, "cpu:fpga", CFG_DEFAULT_CPU_FPGA);

	for (int i=0 ; i<16 ; i++) {
		const char *ch_name = cfg_fgetstr(cfg, "io:channel_%i", i);
		if (ch_name) {
			LOG(L_IO, "Initializing I/O channel %i: %s", i, ch_name);
			io_chan[i] = chan_make(i, ch_name, cfg);
			if (!io_chan[i]) {
				return LOGERR("Failed to initialize channel %i: %s.", i, ch_name);
			}
		}
	}

	return E_OK;
}

// -----------------------------------------------------------------------
void io_shutdown()
{
	LOG(L_IO, "Shutdown I/O");
	for (int c_num=0 ; c_num<IO_MAX_CHAN ; c_num++) {
		struct chan *chan = io_chan[c_num];
		if (chan) {
			LOG(L_IO, "Channel %i: %s", c_num, chan->drv->name);
			chan_destroy(chan);
			io_chan[c_num] = NULL;
		}
	}
}

// -----------------------------------------------------------------------
void io_reset()
{
	for (int c_num=0 ; c_num<IO_MAX_CHAN ; c_num++) {
		struct chan *chan = io_chan[c_num];
		if (chan) {
			chan->drv->reset(chan->obj);
		}
	}
}

// -----------------------------------------------------------------------
void io_get_intspec(int ch, uint16_t *int_spec)
{
	if (io_chan[ch]) {
		io_chan[ch]->drv->cmd(io_chan[ch]->obj, IO_IN, CHAN_CMD_INTSPEC<<10, int_spec);
	}
}

// -----------------------------------------------------------------------
int io_dispatch(int dir, uint16_t n, uint16_t *r)
{
	int is_mem_cmd = n & 1; // 1 = memory configuration, 0 = I/O command
	char narg[64];

	// software memory configuration
	if (is_mem_cmd) {
		if (dir == IO_OU) {
			return mem_cmd(n, *r);
		} else {
			LOG(L_IO, "MEM command shouldn't be IN");
			return IO_NO;
		}
	// channel/unit command
	} else {
		int chan_n = (n & 0b0000000000011110) >> 1;
		struct chan *chan = io_chan[chan_n];
		int res;
		if (LOG_WANTS(L_IO)) {
			int2binf(narg, "cmd: ... .. ...... ch: .... .", n, 16);
			LOG(L_IO, "I/O %s, chan: %d, n_arg: %s (0x%04x), r_arg: 0x%04x", dir ? "fetch" : "send", chan_n, narg, n, *r);
		}

		if (chan) {
			res = chan->drv->cmd(chan->obj, dir, n, r);
		} else {
			res = IO_NO;
		}

		// Bus contains "0" if the IN command fails with EN or times out,
		// and "0" is written to the register.
		if ((dir == IO_IN) && ((res == IO_EN) || (res == IO_NO))) *r = 0;

		LOG(L_IO, "I/O result: %s, r_arg = 0x%04x", io_result_names[res], *r);
		return res;
	}
}

// -----------------------------------------------------------------------
void io_int_set_pa()
{
	if (fpga) {
		iob_pa_send();
	} else {
		int_set(INT_IFACE_POWER);
	}
}

// -----------------------------------------------------------------------
void io_int_set(int x)
{
	if (fpga) {
		iob_int_send(x & 0b1111);
	} else {
		int_set((x & 0b1111) + 12);
	}
}

// -----------------------------------------------------------------------
bool io_mem_read_1(int nb, uint16_t addr, uint16_t *data)
{
	if (fpga) {
		return iob_mem_read_1(nb, addr, data);
	} else {
		return mem_read_1(nb, addr, data);
	}
}

// -----------------------------------------------------------------------
bool io_mem_write_1(int nb, uint16_t addr, uint16_t data)
{
	if (fpga) {
		return iob_mem_write_1(nb, addr, data);
	} else {
		return mem_write_1(nb, addr, data);
	}
}

// -----------------------------------------------------------------------
bool io_mem_read_n(int nb, uint16_t saddr, uint16_t *dest, int count)
{
	if (fpga) {
		return iob_mem_read_n(nb, saddr, dest, count);
	} else {
		return mem_read_n(nb, saddr, dest, count);
	}
}

// -----------------------------------------------------------------------
bool io_mem_write_n(int nb, uint16_t saddr, uint16_t *src, int count)
{
	if (fpga) {
		return iob_mem_read_n(nb, saddr, src, count);
	} else {
		return mem_write_n(nb, saddr, src, count);
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
