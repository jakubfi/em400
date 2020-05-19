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

#define _XOPEN_SOURCE 500

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "log.h"

#include "utils/utils.h"
#include "fpga/iobus.h"
#include "io/io.h"
#include "io/defs.h"
#include "cfg.h"

#define BR 0
#define BW 1

static int xbus;
static int ibus[2];
static int ibusi[2];
static int quit;

pthread_mutex_t bus_mutex;

const char *iob_req_names[] = {
	"PA", "CLEAR", "WRITE", "READ",
	"SEND", "FETCH", "INT",	"e0111",
	"CPD", "CPR", "CPF", "CPS",
	"e1100", "e1101", "e1110", "e1111"
};

const char * iob_resp_names[] = {
	"e0000", "EN", "OK", "PE",
	"NO", "e0101", "e0110", "e0111",
	"e1100", "e1101", "e1110", "e1111"
};

const char *iob_fnkey_names[] = {
	"START", "MODE", "CLOCK", "STOP*N", "STEP", "FETCH", "STORE", "CYCLE",
	"LOAD", "BIN", "OPRQ", "CLEAR", "12", "13", "14", "15"
};

const char *iob_reg_names[] = {
	"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
	"IC", "AC", "AR", "IR", "SR", "RZ", "KB", "KB"
};

// -----------------------------------------------------------------------
int iob_init(em400_cfg *cfg)
{
	if (cfg_getbool(cfg, "cpu:fpga", CFG_DEFAULT_CPU_FPGA) == 0) {
		LOG(L_FPGA, "FPGA IO bus is disabled.");
		return E_OK;
	}

	const char *bus_dev = cfg_getstr(cfg, "fpga:device", CFG_DEFAULT_FPGA_DEVICE);
	int speed = cfg_getint(cfg, "fpga:speed", CFG_DEFAULT_FPGA_SPEED);

	speed_t setspeed = serial_int2speed(speed);
	if (setspeed == 0) {
		return LOGERR("Wrong FPGA bus speed: %i", speed);
	}

	xbus = serial_open(bus_dev, setspeed);

	if (xbus < 0) {
		return LOGERR("Failed to open FPGA bus device: %s", bus_dev);
	}

	if (pthread_mutex_init(&bus_mutex, NULL)) {
		iob_close();
		return LOGERR("Failed to initialize FPGA bus mutex");
	}

	if (pipe(ibus)) {
		iob_close();
		return LOGERR("Failed to initialize internal FPGA bus pipe");
	}

	if (pipe(ibusi)) {
		iob_close();
		return LOGERR("Failed to initialize internal FPGA bus pipe");
	}

	LOG(L_FPGA, "FPGA IO bus initialized. Device: %s, speed: %i", bus_dev, speed);

	return E_OK;
}

// -----------------------------------------------------------------------
void iob_close()
{
	close(xbus);
}

// -----------------------------------------------------------------------
void iob_quit()
{
	atom_store_release(&quit, 1);
}

// -----------------------------------------------------------------------
void iob_msg_log(struct iob_msg *m, char *buf)
{
	int bpos;
	bpos = sprintf(buf, "  %s/%s",
		m->is_req ? "req" : "resp",
		m->is_req ? iob_req_names[m->cmd] : iob_resp_names[m->cmd]
	);

	if (m->is_req) {
		switch (m->cmd) {
		case IOB_CMD_CPF:
			sprintf(buf+bpos, ": %s = %i", iob_fnkey_names[m->nb], m->pn);
			break;
		case IOB_CMD_CPR:
			sprintf(buf+bpos, ": rot = %s", iob_reg_names[m->nb]);
			break;
		case IOB_CMD_CPD:
			sprintf(buf+bpos, ": keys = 0x%04x (%i)", m->dt, m->dt);
			break;
		case IOB_CMD_CPS:
		case IOB_CMD_CL:
			break;
		case IOB_CMD_IN:
			sprintf(buf+bpos, ": channel %i", m->dt>>1);
			break;
		case IOB_CMD_R:
			sprintf(buf+bpos, ": [%i:0x%04x]", m->nb, m->ad);
			break;
		case IOB_CMD_W:
			sprintf(buf+bpos, ": [%i:0x%04x] = 0x%04x", m->nb, m->ad, m->dt);
			break;
		case IOB_CMD_S:
			sprintf(buf+bpos, ": addr: 0x%04x, data: 0x%04x", m->ad, m->dt);
			break;
		case IOB_CMD_F:
			sprintf(buf+bpos, ": addr: 0x%04x", m->ad);
			break;
		default:
			sprintf(buf+bpos, ": argc: %i, args:<%s%s%s>, pn:%i, q:%i, nb:%i, ad:0x%04x, dt:0x%04x (%s)",
				m->b_argc,
				m->has_a1 ? "1" : ".",
				m->has_a2 ? "2" : ".",
				m->has_a3 ? "3" : ".",
				m->pn, m->qb, m->nb,
				m->ad, m->dt,
				m->is_valid ? "valid" : m->invalid_reason
			);
			break;
		}
	} else {
		switch (m->cmd) {
		case IOB_CMD_OK:
			if (m->has_a3) {
				if (m->has_a2) {
					sprintf(buf+bpos, ": data: 0x%04x (%i), rot: %s, leds: %s%s%s%s%s%s%s%s%s%s", m->ad, m->ad, iob_reg_names[m->dt&15],
						m->dt>>6 & IOB_LED_ALARM ? "ALARM " : "",
						m->dt>>6 & IOB_LED_WAIT ? "WAIT " : "",
						m->dt>>6 & IOB_LED_RUN ? "RUN " : "",
						m->dt>>6 & IOB_LED_IRQ ? "IRQ " : "",
						m->dt>>6 & IOB_LED_MC ? "MC " : "",
						m->dt>>6 & IOB_LED_P ? "P " : "",
						m->dt>>6 & IOB_LED_Q ? "Q " : "",
						m->dt>>6 & IOB_LED_CLOCK ? "CLOCK " : "",
						m->dt>>6 & IOB_LED_STOPN ? "STOP*N " : "",
						m->dt>>6 & IOB_LED_MODE ? "MODE " : ""
					);
				} else {
					sprintf(buf+bpos, ": 0x%04x (%i)", m->dt, m->dt);
				}
			}
			break;
		case IOB_CMD_PE:
		case IOB_CMD_NO:
		case IOB_CMD_EN:
			break;
		default:
			sprintf(buf+bpos, ": argc: %i, args:<%s%s%s>, pn:%i, q:%i, nb:%i, ad:0x%04x, dt:0x%04x (%s)",
				m->b_argc,
				m->has_a1 ? "1" : ".",
				m->has_a2 ? "2" : ".",
				m->has_a3 ? "3" : ".",
				m->pn, m->qb, m->nb,
				m->ad, m->dt,
				m->is_valid ? "valid" : m->invalid_reason
			);
			break;
		}
	}
}

// -----------------------------------------------------------------------
static int iob_check_msg(struct iob_msg *m)
{
	if ((m->is_req) && ((m->cmd > 11) || (m->cmd == 7))) {
		m->invalid_reason = strdup("unknown request");
		m->is_valid = 0;
	} else if ((!m->is_req) && ((m->cmd < 1) || (m->cmd > 4))) {
		m->invalid_reason = strdup("unknown response");
		m->is_valid = 0;
	} else if (m->b_argc > 5) {
		m->invalid_reason = strdup("too many argument bytes (>5)");
		m->is_valid = 0;
	} else {
		m->is_valid = 1;
	}

	return m->is_valid;
}

// -----------------------------------------------------------------------
static void iob_update_argc(struct iob_msg *m)
{
	m->b_argc = 0;
	if (m->has_a1) m->b_argc += 1;
	if (m->has_a2) m->b_argc += 2;
	if (m->has_a3) m->b_argc += 2;
}

// -----------------------------------------------------------------------
static struct iob_msg * iob_msg_read(int bus)
{
	uint8_t buf[6];
	int bpos = 0;
	int res;
	int total_recvd = 0;
	int need;

	struct iob_msg *m = (struct iob_msg *) calloc(1, sizeof(struct iob_msg));
	if (!m) {
		goto done;
	}

	// read command
	res = read(bus, buf, 1);
	if (res != 1) {
		LOG(L_FPGA, "ERROR: command read returned: %i", res);
		m->is_valid = 0;
		m->invalid_reason = strdup("failed to read command byte");
		goto done;
	}
	total_recvd += res;

	m->cmd = (buf[bpos] >> 3) & 0b1111;
	m->is_req = (buf[bpos] >> 7) & 1;
	m->has_a1 = (buf[bpos] & 0b100) >> 2;
	m->has_a2 = (buf[bpos] & 0b010) >> 1;
	m->has_a3 = (buf[bpos] & 0b001) >> 0;

	iob_update_argc(m);

	if (m->cmd == IOB_CMD_S) {
		m->io_dir = IO_OU;
	} else if (m->cmd == IOB_CMD_F) {
		m->io_dir = IO_IN;
	} else {
		m->io_dir = -1;
	}

	bpos += 1;

	if (!iob_check_msg(m)) {
		goto done;
	}

	// read arguments

	// TODO: retries/timeout ?
	need = m->b_argc;
	while (need > 0) {
		res = read(bus, buf+bpos+m->b_argc-need, need);
		if (res <= 0) {
			LOG(L_FPGA, "ERROR: argument read returned: %i, need to read %i more bytes", res, need);
		} else {
			need -= res;
		}
		total_recvd += res;
	}

	if (m->has_a1) {
		m->nb = buf[bpos] & 0b1111;
		m->pn = (buf[bpos] >> 4) & 1;
		m->qb = (buf[bpos] >> 5) & 1;
		bpos += 1;
	}
	if (m->has_a2) {
		m->ad = (buf[bpos] << 8) + buf[bpos+1];
		bpos += 2;
	}
	if (m->has_a3) {
		m->dt = (buf[bpos] << 8) + buf[bpos+1];
		bpos += 2;
	}

done:
	if (bus == xbus) {
		char lbuf1[1024];
		char lbuf2[64];
		iob_msg_log(m, lbuf1);
		for (int i=0 ; i<total_recvd ; i++) {
			sprintf(lbuf2+3*i, " %02x", buf[i]);
		}
		LOG(L_FPGA, "%s%s ::%s", m->is_valid ? "" : "ERROR ", lbuf1, lbuf2);
		if (!m->is_valid) {
			LOG(L_FPGA, "Message invalid: %s", m->invalid_reason);
		}
	}
	return m;
}

// -----------------------------------------------------------------------
int iob_msg_send(int bus, struct iob_msg *m)
{
	uint8_t buf[6];
	int res = 0;
	int bpos = 0;

	buf[bpos] = (m->is_req << 7) | (m->cmd << 3) | (m->has_a1 << 2) | (m->has_a2 << 1) | (m->has_a3 << 0);
	bpos++;

	if (m->has_a1) {
		buf[bpos] = (m->qb << 5) | (m->pn << 4) | m->nb;
		bpos += 1;
	}
	if (m->has_a2) {
		buf[bpos] = m->ad >> 8;
		buf[bpos+1] = m->ad & 0xff;
		bpos += 2;
	}
	if (m->has_a3) {
		buf[bpos] = m->dt >> 8;
		buf[bpos+1] = m->dt & 0xff;
	}

	iob_update_argc(m);
	iob_check_msg(m);

	if (bus == xbus) {
		char lbuf1[1024];
		char lbuf2[64];
		iob_msg_log(m, lbuf1);
		for (int i=0 ; i<1+m->b_argc ; i++) {
			sprintf(lbuf2+3*i, " %02x", buf[i]);
		}
		LOG(L_FPGA, "%s%s ::%s", m->is_valid ? "" : "ERROR ", lbuf1, lbuf2);
		if (!m->is_valid) {
			LOG(L_FPGA, "Message invalid: %s", m->invalid_reason);
		}
	}

	if (m->is_valid) {
		int need = 1+m->b_argc;
		while (need > 0) {
			res = write(bus, buf, need);
			if (res <= 0) {
				LOG(L_FPGA, "ERROR: write returned: %i, need to write %i more bytes", res, need);
			} else {
				need -= res;
			}
		}
		tcdrain(bus);
	}

	return res;
}

// -----------------------------------------------------------------------
void iob_msg_free(struct iob_msg *msg)
{
	free(msg);
}

// -----------------------------------------------------------------------
static struct iob_msg * iob_dialog(struct iob_msg* mo)
{
	struct iob_msg *mi = NULL;
	fd_set fds;
	struct timeval timeout;

	pthread_mutex_lock(&bus_mutex);

	iob_msg_send(ibus[BW], mo);
	//LOG(L_FPGA, "Client is waiting for reply");

	FD_ZERO(&fds);
	FD_SET(ibusi[BR], &fds);
	timeout.tv_sec = 0;
	timeout.tv_usec = 1000 * 1000;

	int select_res = select(ibusi[BR]+1, &fds, NULL, NULL, &timeout);
	if (select_res > 0) {
		mi = iob_msg_read(ibusi[BR]);
	} else {
		LOG(L_FPGA, "select() returned %i", select_res);
		// fake reply so main MX loop does not stuck waiting for reset to end
		mi = (struct iob_msg *) calloc(1, sizeof(struct iob_msg));
		mi->cmd = IOB_CMD_OK;
	}

	pthread_mutex_unlock(&bus_mutex);

	return mi;
}

// -----------------------------------------------------------------------
void iob_loop()
{
	int io_res;
	struct iob_msg *mi;
	struct iob_msg *mo;
	struct iob_msg *intreq = NULL;
	fd_set fds;
	struct timeval timeout;
	static struct timeval xt1, xt2;

	while (!atom_load_acquire(&quit)) {
		FD_ZERO(&fds);
		FD_SET(xbus, &fds);
		FD_SET(ibus[BR], &fds);
		timeout.tv_sec = 0;
		timeout.tv_usec = 100 * 1000;

		int select_res = select(ibus[BR]+1, &fds, NULL, NULL, &timeout);
		if (select_res > 0) {

			// message on the external bus
			if (FD_ISSET(xbus, &fds)) {
				mi = iob_msg_read(xbus);
				if (!mi->is_valid) {
					LOG(L_FPGA, "ERROR: Message invalid, ignoring: ", mi->invalid_reason);
					iob_msg_free(mi);
					continue;
				}
				if (mi->is_req) {
					switch (mi->cmd) {
						case IOB_CMD_CL:
							io_reset();
							break;
						case IOB_CMD_S:
						case IOB_CMD_F:
							io_res = io_dispatch(mi->io_dir, mi->ad, &mi->dt);
							if (io_res != IO_NO) {
								gettimeofday(&xt1, NULL);
								iob_reply_send(xbus, mi, io_res);
								gettimeofday(&xt2, NULL);
								double elapsed_us = 1000000.0 * (xt2.tv_sec - xt1.tv_sec) + (xt2.tv_usec - xt1.tv_usec);
								LOG(L_FPGA, "External request service time: %.0f us", elapsed_us);
							}
							break;
						default:
							LOG(L_FPGA, "ERROR: Not an I/O request, ignored");
							break;
					}
				} else {
					switch (mi->cmd) {
						case IOB_CMD_NO:
						case IOB_CMD_OK:
						case IOB_CMD_PE:
							if (intreq) {
								iob_msg_send(ibusi[BW], mi);
								free(intreq);
								intreq = NULL;
							} else {
								LOG(L_FPGA, "ERROR: Noone waiting for the reply, ignored");
							}
							break;
						default:
							LOG(L_FPGA, "ERROR: Not an I/O reply, ignored");
							break;
					}

				}

				iob_msg_free(mi);

			// message on the internal bus
			} else if (FD_ISSET(ibus[BR], &fds)) {
				intreq = iob_msg_read(ibus[BR]);
				if (!intreq->is_valid) {
					LOG(L_FPGA, "ERROR: Message invalid, ignoring: %s", intreq->invalid_reason);
					iob_msg_free(intreq);
					intreq = NULL;
					continue;
				}

				switch (intreq->cmd) {
					case IOB_CMD_R:
					case IOB_CMD_W:
					case IOB_CMD_IN:
					case IOB_CMD_CPS:
						iob_msg_send(xbus, intreq);
						break;
					case IOB_CMD_CPD:
					case IOB_CMD_CPR:
					case IOB_CMD_CPF:
					case IOB_CMD_PA:
						iob_msg_send(xbus, intreq);
						free(intreq);
						intreq = NULL;
						// fake reply
						mo = (struct iob_msg *) calloc(1, sizeof(struct iob_msg));
						mo->cmd = IOB_CMD_OK;
						iob_msg_send(ibusi[BW], mo);
						free(mo);
						break;
					default:
						LOG(L_FPGA, "ERROR: Not an I/O request nor reply, ignored");
						free(intreq);
						intreq = NULL;
						break;
				}
			} else {
				LOG(L_FPGA, "ERROR: No know fd");
			}
		}
	}
}

// -----------------------------------------------------------------------
void iob_reply_send(int bus, struct iob_msg *mi, int io_res)
{
	if (!mi->is_req && (io_res == 0)) {
		// no need to send 'IO_NO' reply, CPU handles it with a timeout
		return;
	}

	struct iob_msg *mo = (struct iob_msg *) calloc(1, sizeof(struct iob_msg));

	mo->cmd = io_res;

	if ((mi->cmd == IOB_CMD_F) && (io_res == IO_OK)) {
		mo->has_a3 = 1;
		mo->dt = mi->dt;
	}
	iob_msg_send(bus, mo);

	iob_msg_free(mo);
}

// -----------------------------------------------------------------------
void iob_cp_set_keys(uint16_t k)
{
	struct iob_msg *mo = (struct iob_msg *) calloc(1, sizeof(struct iob_msg));

	mo->cmd = IOB_CMD_CPD;
	mo->is_req = 1;
	mo->has_a3 = 1;
	mo->dt = k;

	iob_dialog(mo);

	iob_msg_free(mo);
}

// -----------------------------------------------------------------------
void iob_cp_set_rotary(int r)
{
	struct iob_msg *mo = (struct iob_msg *) calloc(1, sizeof(struct iob_msg));

	mo->cmd = IOB_CMD_CPR;
	mo->is_req = 1;
	mo->has_a1 = 1;
	mo->nb = r;

	iob_dialog(mo);

	iob_msg_free(mo);
}

// -----------------------------------------------------------------------
void iob_cp_set_fn(int fn, int v)
{
	struct iob_msg *mo = (struct iob_msg *) calloc(1, sizeof(struct iob_msg));

	mo->cmd = IOB_CMD_CPF;
	mo->is_req = 1;
	mo->has_a1 = 1;
	mo->nb = fn;
	mo->pn = v;

	iob_dialog(mo);

	iob_msg_free(mo);
}

// -----------------------------------------------------------------------
struct iob_cp_status * iob_cp_get_status()
{
	struct iob_msg *mo = (struct iob_msg *) calloc(1, sizeof(struct iob_msg));

	mo->cmd = IOB_CMD_CPS;
	mo->is_req = 1;

	struct iob_msg *mi = iob_dialog(mo);

	struct iob_cp_status *stat = (struct iob_cp_status *) malloc(sizeof(struct iob_cp_status));
	stat->data = mi->ad;
	stat->rot = mi->dt & 0b1111;
	stat->leds = mi->dt >> 6;

	iob_msg_free(mi);
	iob_msg_free(mo);

	return stat;
}

// -----------------------------------------------------------------------
void iob_int_send(int x)
{
	struct iob_msg *mo = (struct iob_msg *) calloc(1, sizeof(struct iob_msg));

	mo->cmd = IOB_CMD_IN;
	mo->is_req = 1;
	mo->has_a1 = 1;
	mo->has_a3 = 1;
	mo->pn = 0; // TODO: 2cpu support
	mo->dt = (x & 0b1111) << 1;

	struct iob_msg *mi = iob_dialog(mo);

	iob_msg_free(mi);
	iob_msg_free(mo);
}

// -----------------------------------------------------------------------
void iob_pa_send()
{
	struct iob_msg *mo = (struct iob_msg *) calloc(1, sizeof(struct iob_msg));

	mo->cmd = IOB_CMD_PA;
	mo->is_req = 1;

	iob_dialog(mo);

	iob_msg_free(mo);

}

// -----------------------------------------------------------------------
int iob_mem_get(int nb, uint16_t addr, uint16_t *data)
{
	int ret;
	struct iob_msg *mo = (struct iob_msg *) calloc(1, sizeof(struct iob_msg));

	mo->cmd = IOB_CMD_R;
	mo->is_req = 1;
	mo->has_a1 = 1;
	mo->has_a2 = 1;
	mo->pn = 0; // TODO: 2cpu support
	mo->nb = nb;
	mo->ad = addr;

	struct iob_msg *mi = iob_dialog(mo);

	if (mi->cmd == IOB_CMD_OK) {
		*data = mi->dt;
		ret = 1;
	} else {
		ret = 0;
	}

	iob_msg_free(mi);
	iob_msg_free(mo);

	return ret;
}

// -----------------------------------------------------------------------
int iob_mem_put(int nb, uint16_t addr, uint16_t data)
{
	int ret;
	struct iob_msg *mo = (struct iob_msg *) calloc(1, sizeof(struct iob_msg));

	mo->cmd = IOB_CMD_W;
	mo->is_req = 1;
	mo->has_a1 = 1;
	mo->has_a2 = 1;
	mo->has_a3 = 1;
	mo->pn = 0; // TODO: 2cpu support
	mo->nb = nb;
	mo->ad = addr;
	mo->dt = data;

	struct iob_msg *mi = iob_dialog(mo);

	if (mi->cmd == IOB_CMD_OK) {
		ret = 1;
	} else {
		ret = 0;
	}

	iob_msg_free(mi);
	iob_msg_free(mo);

	return ret;
}

// -----------------------------------------------------------------------
int iob_mem_mget(int nb, uint16_t saddr, uint16_t *dest, int count)
{
	int ret;
	int words = 0;
	int cnt = 0;
	uint16_t d;

	while (cnt < count) {
		ret = iob_mem_get(nb, saddr+cnt, &d);
		if (!ret) break;
		*(dest+cnt) = d;
		words += ret;
		cnt++;
	}

	return words;
}

// -----------------------------------------------------------------------
int iob_mem_mput(int nb, uint16_t saddr, uint16_t *src, int count)
{
	int ret;
	int words = 0;
	int cnt = 0;

	while (cnt < count) {
		ret = iob_mem_put(nb, saddr+cnt, *(src+cnt));
		if (!ret) break;
		words += ret;
		cnt++;
	}

	return words;
}

// vim: tabstop=4 shiftwidth=4 autoindent
