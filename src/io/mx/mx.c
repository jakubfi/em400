//  Copyright (c) 2012-2018 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#include "atomic.h"
#include "log.h"
#include "utils/elst.h"
#include "io/io.h"
#include "io/dev/dev.h"
#include "io/chan.h"
#include "io/mx/mx.h"
#include "io/mx/cmds.h"
#include "io/mx/irq.h"
#include "io/mx/event.h"
#include "io/mx/line.h"
#include "cfg.h"

// Doing asynchronous reset that mimics hardware is hard in multithreaded software.
// Trick used here is as follows:
//  * Upon receiving a reset request, set the state to MX_UNINITIALIZED and let everything run, but...
//  * cut all communication between multix and the CPU/memory. That means:
//     * every command received from the CPU thread (except RESET and QUIT) will fail
//     * every memory access initiated by multix will fail
//     * no interrupt will be sent from multix to the CPU
// This way threads can continue, but any processing that stated just before the reset will have no effect.
enum mx_condition { MX_UNINITIALIZED, MX_INITIALIZED, MX_CONFIGURED, MX_QUIT };

// Real multix boots up in probably just under a second
// (~500ms for ROM/RAM check + ~185ms for RAM cleanup).
// Here we need just a reasonable delay - big enough for
// OS scheduler to switch threads between MULTIX and CPU,
// so we don't finish MULTIX' job before switching back to CPU thread.
#define MX_INIT_TIME_MSEC 150

typedef int (*mx_cmd_fun)(struct mx *multix, int log_n, uint16_t arg);

static void * mx_event_loop(void *ptr);
static int mx_event(struct mx *multix, int type, int cmd, int log_n, uint16_t r_arg);
int mx_int_enqueue(struct mx *multix, int intr, int line);
void mx_cmd_reset(void *ch);

// -----------------------------------------------------------------------
void mx_event_destructor(void *ptr)
{
	free(ptr);
}

// -----------------------------------------------------------------------
void * mx_create(int ch_num, em400_cfg *cfg)
{
	LOG(L_MX, "Creating new MULTIX");

	// --- create multix itself (everything needs it)

	struct mx *multix = (struct mx *) calloc(1, sizeof(struct mx));
	if (!multix) {
		LOGERR("Memory allocation error.");
		goto cleanup;
	}
	// initialize multix structure
	multix->chnum = ch_num;
	atom_store_release(&multix->state, MX_UNINITIALIZED);
	for (int i=0 ; i<MX_LINE_CNT ; i++) {
		struct mx_line *pline = multix->plines + i;
		pline->phy_n = i;
		pline->multix = multix;
		pline->protoq = elst_create(1024, mx_event_destructor);
		pline->statusq = elst_create(1024, mx_event_destructor);
		if (pthread_mutex_init(&pline->status_mutex, NULL)) {
			LOGERR("Failed to initialize line %i status mutex.", i);
			goto cleanup;
		}
	}

	// --- create interrupt system (devices need it)

	if (pthread_mutex_init(&multix->int_mutex, NULL)) {
		LOGERR("Failed to initialize interrupt mutex.");
		goto cleanup;
	}
	multix->intq = elst_create(1024, mx_event_destructor);
	if (!multix->intq) {
		LOGERR("Failed to create interrupt queue.");
		goto cleanup;
	}

	// --- create devices (event system needs them)

	LOG(L_MX, "Initializing devices");
	for (int pline_num=0 ; pline_num<MX_LINE_CNT ; pline_num++) {
		if (!cfg_fcontains(cfg, "dev%i.%i", ch_num, pline_num)) continue;

		struct mx_line *line = multix->plines + pline_num;
		int res = dev_make(cfg, ch_num, pline_num, &line->dev, &line->dev_data);
		if (res != E_OK) {
			LOGERR("Failed to create MULTIX device: %i.%i", ch_num, pline_num);
			goto cleanup;
		}
	}

	// --- create event system (MERA-400 interface needs it)

	multix->eventq = elst_create(1024, mx_event_destructor);
	if (!multix->eventq) {
		LOGERR("Failed to create event queue.");
		goto cleanup;
	}
	if (pthread_create(&multix->ev_thread, NULL, mx_event_loop, multix)) {
		LOGERR("Failed to spawn event processor thread.");
		goto cleanup;
	}

	char name[16];
	snprintf(name, 15, "mxev%02i", multix->chnum);
	pthread_setname_np(multix->ev_thread, name);

	LOG(L_MX, "MULTIX created");

	return multix;

cleanup:
	if (multix) {
		// --- destroy event system
		elst_destroy(multix->eventq);
		// --- destroy devices
		for (int i=0 ; i<MX_LINE_CNT ; i++) {
			struct mx_line *pline = multix->plines + i;
			if (pline->dev) {
				pline->dev->destroy(pline->dev_data);
			}
		}
		// --- destroy interrupt system
		elst_destroy(multix->intq);
		pthread_mutex_destroy(&multix->int_mutex);
		// --- destroy multix itself
		for (int i=0 ; i<MX_LINE_CNT ; i++) {
			struct mx_line *pline = multix->plines + i;
			elst_destroy(pline->protoq);
			elst_destroy(pline->statusq);
			pthread_mutex_destroy(&pline->status_mutex);
		}
		free(multix);
	}

	return NULL;
}

// -----------------------------------------------------------------------
static void mx_lines_deinit(struct mx *multix)
{
	LOG(L_MX, "Deinitializing logical lines");

	// send QUIT event to all line threads
	for (int i=0 ; i<MX_LINE_CNT ; i++) {
		struct mx_line *lline = multix->llines[i];
		if (lline) {
			// quit the protocol thread
			struct mx_event *ev = (struct mx_event *) malloc(sizeof(struct mx_event));
			if (ev) {
				ev->type = MX_EV_QUIT;
				if (elst_insert(lline->protoq, ev, MX_EV_QUIT) > 0) {
					pthread_join(lline->proto_th, NULL);
				} else {
					LOG(L_MX, "Failed to send QUIT event to %s protocol queue on line %i, terminating event thread", lline->proto->name, lline->log_n );
					pthread_cancel(lline->proto_th);
				}
			}
			// quit the status thread
			struct mx_event *ev2 = (struct mx_event *) malloc(sizeof(struct mx_event));
			if (ev2) {
				ev2->type = MX_EV_QUIT;
				if (elst_insert(lline->statusq, ev2, MX_EV_QUIT) > 0) {
					pthread_join(lline->status_th, NULL);
				} else {
					LOG(L_MX, "Failed to send QUIT event to %s status queue on line %i, terminating event thread", lline->proto->name, lline->log_n );
					pthread_cancel(lline->status_th);
				}
			}
			lline->log_n = -1;
			lline->status = MX_LSTATE_NONE;
			multix->llines[i] = NULL;
		}
	}

	LOG(L_MX, "Deinitializing physical lines");

	for (int i=0 ; i<MX_LINE_CNT ; i++) {
		struct mx_line *pline = multix->plines + i;
		pline->dir = 0;
		pline->type = 0;
		pline->used = 0;
		if (pline->proto) {
			pline->proto->destroy(pline);
			pline->proto = NULL;
			pline->proto_data = NULL;
		}
	}
}

// -----------------------------------------------------------------------
void mx_shutdown(void *ch)
{
	if (!ch) return;

	LOG(L_MX, "Multix shutting down");

	struct mx *multix = (struct mx *) ch;

	// --- make multix uninitialized (further interface commands will be dropped)

	atom_store_release(&multix->state, MX_UNINITIALIZED);

	// --- destroy event system

	mx_event(multix, MX_EV_QUIT, 0, 0, 0);
	pthread_join(multix->ev_thread, NULL);
	elst_destroy(multix->eventq);

	// --- deinit lines, destroy devices

	mx_lines_deinit(multix);

	// --- destroy interrupt system

	elst_destroy(multix->intq);
	pthread_mutex_destroy(&multix->int_mutex);

	// --- destroy multix itself

	for (int i=0 ; i<MX_LINE_CNT ; i++) {
		struct mx_line *pline = multix->plines + i;
		if (pline->dev) {
			pline->dev->destroy(pline->dev_data);
			pline->dev = NULL;
			pline->dev_data = NULL;
		}
		elst_destroy(pline->protoq);
		elst_destroy(pline->statusq);
		pthread_mutex_destroy(&pline->status_mutex);
	}
	free(multix);
	LOG(L_MX, "Shutdown complete");
}

// -----------------------------------------------------------------------
int mx_mem_mget(struct mx *multix, int nb, uint16_t addr, uint16_t *data, int len)
{
	if (atom_load_acquire(&multix->state) == MX_UNINITIALIZED) {
		LOG(L_MX, "LOST memory read due to multix initializing");
		return 0;
	}

	if (io_mem_mget(nb, addr, data, len) != len) {
		return -1;
	}
	return 0;
}

// -----------------------------------------------------------------------
int mx_mem_mput(struct mx *multix, int nb, uint16_t addr, uint16_t *data, int len)
{
	if (atom_load_acquire(&multix->state) == MX_UNINITIALIZED) {
		LOG(L_MX, "LOST memory write due to multix initializing");
		return 0;
	}

	if (io_mem_mput(nb, addr, data, len) != len) {
		return -1;
	}
	return 0;
}

// -----------------------------------------------------------------------
static void mx_int_set(struct mx *multix)
{
	if (atom_load_acquire(&multix->state) == MX_UNINITIALIZED) {
		LOG(L_MX, "LOST interrupt due to multix initializing");
		return;
	}

	io_int_set(multix->chnum);
}

// -----------------------------------------------------------------------
static void mx_int_push(struct mx *multix)
{
	int send = 0;

	pthread_mutex_lock(&multix->int_mutex);
	if (multix->intspec == MX_IRQ_INIEA) {
		uint16_t *i = (uint16_t *) elst_nlock_pop(multix->intq);
		if (i) {
			multix->intspec = *i;
			send = 1;
			free(i);
		}
	}
	pthread_mutex_unlock(&multix->int_mutex);

	if (send) {
		LOG(L_MX, "Pushing interrupt to CPU");
		mx_int_set(multix);
	}
}

// -----------------------------------------------------------------------
int mx_int_enqueue(struct mx *multix, int intr, int line)
{
	LOG(L_MX, "Enqueue interrupt %i (%s), line %i", intr, mx_irq_name(intr), line);

	// TODO: this is silly
	uint16_t *i = (uint16_t *) malloc(sizeof(uint16_t));
	*i = (intr << 8) | line;

	pthread_mutex_lock(&multix->int_mutex);
	int res = elst_nlock_append(multix->intq, i);
	pthread_mutex_unlock(&multix->int_mutex);

	mx_int_push(multix);

	return res;
}

// -----------------------------------------------------------------------
static uint16_t mx_int_get_spec(struct mx *multix)
{
	uint16_t lintspec;

	pthread_mutex_lock(&multix->int_mutex);
	lintspec = multix->intspec;
	multix->intspec = MX_IRQ_INIEA;
	pthread_mutex_unlock(&multix->int_mutex);

	LOG(L_MX, "Sending intspec to CPU: 0x%04x (%s, line %i)", lintspec, mx_irq_name(lintspec>>8), lintspec & 0xFF);

	return lintspec;
}

// -----------------------------------------------------------------------
void mx_int_reset(struct mx *multix)
{
	pthread_mutex_lock(&multix->int_mutex);
	multix->intspec = MX_IRQ_INIEA;
	elst_nlock_clear(multix->intq);
	pthread_mutex_unlock(&multix->int_mutex);
}

// -----------------------------------------------------------------------
static int mx_line_conf_phy(struct mx *multix, int phy_n, uint16_t data)
{
	unsigned dir  = (data & 0b1110000000000000) >> 13;
	unsigned used = (data & 0b0001000000000000) >> 12;
	unsigned type = (data & 0b0000111100000000) >> 8;

	LOG(L_MX, "%s (type %i), %s (%i), %s",
		mx_line_type_name(type),
		type,
		mx_line_dir_name(dir),
		dir, used ? "used" : "unused"
	);

	// check type for correctness
	if (type >= MX_PHY_CNT) {
		return MX_SC_E_DEVTYPE;
	// check direction: USART
	} else if ((type == MX_PHY_USART_SYNC) || (type == MX_PHY_USART_ASYNC)) {
		if ((dir != MX_DIR_OUTPUT) && (dir != MX_DIR_INPUT) && (dir != MX_DIR_HALF_DUPLEX) && (dir != MX_DIR_FULL_DUPLEX)) {
			if (used) {
				return MX_SC_E_DIR;
			} else if (dir != MX_DIR_NONE) {
				return MX_SC_E_DIR;
			}
		}
	// check direction: 8255
	} else if (type == MX_PHY_8255) {
		if ((dir != MX_DIR_OUTPUT) && (dir != MX_DIR_INPUT)) {
			return MX_SC_E_DIR;
		}
	// check direction: winchester, floppy, tape
	} else {
		if (dir != MX_DIR_NONE) {
			return MX_SC_E_DIR;
		}
	}

	multix->plines[phy_n].dir = dir;
	multix->plines[phy_n].used = used;
	multix->plines[phy_n].type = type;

	return MX_SC_E_OK;
}

// -----------------------------------------------------------------------
static int mx_line_conf_log(struct mx *multix, int phy_n, int log_n, uint16_t *data)
{
	unsigned proto_num	= (data[0] & 0b1111111100000000) >> 8;
	// formatter number is not really used anywhere in emulation,
	// but let's at least log its number
	unsigned tape_fmter	= (data[0] & 0b0000000010000000) >> 7;

	struct mx_line *pline = multix->plines + phy_n;
	const struct mx_proto *proto = mx_proto_get(proto_num);

	LOG(L_MX, "Logical line %i is physical line %i, protocol %i: %s%s",
		log_n,
		phy_n,
		proto_num,
		proto ? proto->name : "[unknown]",
		(proto_num == MX_PROTO_MTAPE) ? (tape_fmter ? ", formatter 1" : ", formatter 0" ) : ""
	);

	// make sure physical line is active (configured)
	if (!pline->used) {
		return MX_SC_E_PHY_UNUSED;
	}

	// make sure that no logical line uses this physical line
	for (int i=0 ; i<MX_LINE_CNT ; i++) {
		if (multix->llines[i] == pline) {
			return MX_SC_E_PHY_USED;
		}
	}

	// check if protocol exists and its emulation is usable
	if (!proto) {
		return MX_SC_E_PROTO_MISSING;
	}

	// check if line direction matches required protocol direction
	if ((proto->dir & pline->dir) != proto->dir) {
		return MX_SC_E_DIR_MISMATCH;
	}

	// check if line type is OK for the protocol
	const int *type = proto->phy_types;
	while (*type != pline->type) {
		if (*type == -1) {
			return MX_SC_E_PROTO_MISMATCH;
		}
		type++;
	}

	int res = proto->init(pline, data+1);
	if (res != MX_SC_E_OK) {
		return res;
	}

	pline->log_n = log_n;
	pline->proto = proto;
	multix->llines[log_n] = pline;

	elst_clear(pline->protoq);

	char thname[16];
	snprintf(thname, 15, "mxl%02i:%02i", multix->chnum, pline->log_n);
	if (pthread_create(&pline->proto_th, NULL, mx_line_thread, pline)) {
		return MX_SC_E_NOMEM;
	}
	pthread_setname_np(pline->proto_th, thname);

	snprintf(thname, 15, "mxs%02i:%02i", multix->chnum, pline->log_n);
	if (pthread_create(&pline->status_th, NULL, mx_line_status_thread, pline)) {
		return MX_SC_E_NOMEM;
	}
	pthread_setname_np(pline->status_th, thname);

	return MX_SC_E_OK;
}

// -----------------------------------------------------------------------
static int mx_cmd_setcfg(struct mx *multix, uint16_t addr)
{
#define CFGERR(err, line) ret_int = MX_IRQ_INKON; ret_err = (((err)<<8) | (line))

	int res;
	uint16_t data[MX_LINE_CNT+4*MX_LINE_CNT] = { 0 };

	int ret_int;
	uint16_t ret_err = 0;

	unsigned phy_desc_count;
	unsigned log_count;
	int read_size;
	int cur_line;
	int tape_formatters;

	// check if configuration isn't set already
	if (atom_load_acquire(&multix->state) == MX_CONFIGURED) {
		CFGERR(MX_SC_E_CONFSET, 0);
		goto fail;
	}

	// read configuration header
	if (mx_mem_mget(multix, 0, addr, data, 1)) {
		ret_int = MX_IRQ_INKOT;
		goto fail;
	}

	phy_desc_count = (data[0] & 0b1111111100000000) >> 8;
	log_count      = (data[0] & 0b0000000011111111);

	LOG(L_MX, "Configuration for MULTIX on channel %i has %i physical line descriptors, %i logical lines", multix->chnum, phy_desc_count, log_count);

	// read line descriptions
	read_size = phy_desc_count + 4*log_count;
	if (mx_mem_mget(multix, 0, addr+2, data, read_size)) {
		ret_int = MX_IRQ_INKOT;
		goto fail;
	}

	// check if number of phy line descriptors and log line counts are OK
	if ((phy_desc_count <= 0) || (phy_desc_count > MX_LINE_CNT) || (log_count <= 0) || (log_count > MX_LINE_CNT)) {
		CFGERR(MX_SC_E_NUMLINES, 0);
		goto fail;
	}

	// configure physical lines
	cur_line = 0;
	for (unsigned i=0 ; i<phy_desc_count ; i++) {
		unsigned count = (data[i] & 0b11111) + 1;
		LOG(L_MX, "%i Physical line(-s) %i..%i:", count, cur_line, cur_line+count-1);
		for (unsigned j=0 ; j<count ; j++, cur_line++) {
			if (cur_line >= MX_LINE_CNT) {
				CFGERR(MX_SC_E_NUMLINES, 0);
				goto fail;
			}
			res = mx_line_conf_phy(multix, cur_line, data[i]);
			if (res != MX_SC_E_OK) {
				CFGERR(res, cur_line);
				goto fail;
			}
		}
	}

	// check the completness of physical lines configuration
	tape_formatters = 0;
	for (unsigned i=0 ; i<MX_LINE_CNT ; i+=4) {
		// there can be only one tape formatter (4 lines)
		if ((multix->plines[i].type == MX_PHY_MTAPE) && (++tape_formatters > 1)) {
			CFGERR(MX_SC_E_PHY_INCOMPLETE, i);
			goto fail;
		}
		// MULTIX lines are physically organized in 4-line groups and configuration needs to reflect this
		for (unsigned j=1 ; j<=3 ; j++) {
			if (multix->plines[i+j].type != multix->plines[i].type) {
				CFGERR(MX_SC_E_PHY_INCOMPLETE, i+j);
				goto fail;
			}
		}
	}

	// configure logical lines
	for (unsigned i=0 ; i<log_count ; i++) {
		uint16_t *log_data = data + phy_desc_count + i*4;
		unsigned phy_num = log_data[0] & 0b11111;
		res = mx_line_conf_log(multix, phy_num, i, log_data);
		if (res != MX_SC_E_OK) {
			CFGERR(res, i);
			goto fail;
		}
	}

	ret_int = MX_IRQ_IUKON;

	atom_store_release(&multix->state, MX_CONFIGURED);
	LOG(L_MX, "Multix configuration is now ready");

fail:
	// update return field only if setcfg failed
	if (ret_int == MX_IRQ_INKON) {
		LOG(L_MX, "Configuration error: %s", mx_line_sc_err_name(ret_err>>8));
		// clear lines configuration only if setcfg tried to configure something
		// and failed, not when configuration is already properly set
		if (ret_err != MX_SC_E_CONFSET) {
			mx_lines_deinit(multix);
		}
		if (mx_mem_mput(multix, 0, addr+1, &ret_err, 1)) {
			ret_int = MX_IRQ_INKOT;
		}
	}
	mx_int_enqueue(multix, ret_int, 0);
	return 0;
}

// -----------------------------------------------------------------------
static int mx_cmd_test(struct mx *multix)
{
	if (atom_load_acquire(&multix->state) == MX_QUIT) {
		LOG(L_MX, "Test request ignored, Multix is shutting down");
		return 0;
	}
	// As we're not able to run any real 8085 code, TEST command
	// won't work. Best we can do is to pretend that test is done
	// and let the test wrapper on CPU side worry about the (non-)results.
	// TODO: I think the real multix would self-reset after test finish...
	mx_int_enqueue(multix, MX_IRQ_IWYTE, 0);
	return 0;
}

// -----------------------------------------------------------------------
static int mx_cmd_requeue(struct mx *multix)
{
	pthread_mutex_lock(&multix->int_mutex);
	if (multix->intspec != MX_IRQ_INIEA) {
		uint16_t *i = (uint16_t *) malloc(sizeof(uint16_t));
		*i = multix->intspec;
		// TODO: handle queue full
		elst_nlock_prepend(multix->intq, i);
		multix->intspec = MX_IRQ_INIEA;
	}
	pthread_mutex_unlock(&multix->int_mutex);

	mx_int_push(multix);

	return 0;
}

// -----------------------------------------------------------------------
static void mx_cmd_dispatch(struct mx *multix, struct mx_line *lline, struct mx_event *ev)
{
	// is multix and line configured?
	if (!lline) {
		LOG(L_MX, "(EV%04x) Rejecting command, line %i not configured", ev->log_n, ev->id);
		mx_int_enqueue(multix, mx_irq_noline(ev->cmd), ev->log_n);
		return;
	}

	const struct mx_cmd *cmd = lline->proto->cmd + ev->cmd;

	if (!cmd->run && (ev->cmd != MX_CMD_STATUS)) { // NOTE: 'status' command is not a protocol command
		LOG(L_MX, "(EV%04x) Rejecting command: no protocol function to handle command %s for protocol %s in line %i", ev->id, mx_get_cmd_name(ev->cmd), lline->proto->name, lline->log_n);
		mx_int_enqueue(lline->multix, mx_irq_reject(ev->cmd), ev->log_n);
		return;
	}

	// can line process the command?
	pthread_mutex_lock(&lline->status_mutex);
	if (mx_line_cmd_allowed(lline, ev->cmd)) {
		log_line_status("Rejecting command, line status does not allow execution", ev->log_n, lline->status, ev->id);
		mx_int_enqueue(lline->multix, mx_irq_reject(ev->cmd), ev->log_n);
		pthread_mutex_unlock(&lline->status_mutex);
		return;
	}
	// update line status
	lline->status |= mx_cmd_state(ev->cmd);
	pthread_mutex_unlock(&lline->status_mutex);

	// duplicate the event, as the original gets deleted in event loop
	struct mx_event *ev_dup = malloc(sizeof(struct mx_event));
	memcpy(ev_dup, ev, sizeof(struct mx_event));

	// process asynchronously in the protocol thread
	LOG(L_MX, "(EV%04x) Enqueue command %s for %s in line %i", ev_dup->id, mx_get_cmd_name(ev_dup->cmd), lline->proto->name, lline->log_n);
	if (ev->cmd == MX_CMD_STATUS) {
		elst_append(lline->statusq, ev_dup);
	} else {
		elst_append(lline->protoq, ev_dup);
	}
}

// -----------------------------------------------------------------------
static void mx_process_cmd_event(struct mx *multix, struct mx_event *ev)
{
	struct mx_line *lline;

	switch (ev->cmd) {
		case MX_CMD_REQUEUE:
			mx_cmd_requeue(multix);
			break;
		case MX_CMD_STATUS:
		case MX_CMD_TRANSMIT:
		case MX_CMD_ATTACH:
		case MX_CMD_DETACH:
		case MX_CMD_ABORT:
			lline = multix->llines[ev->log_n];
			mx_cmd_dispatch(multix, lline, ev);
			break;
		case MX_CMD_SETCFG:
			mx_cmd_setcfg(multix, ev->arg);
			break;
		case MX_CMD_TEST:
			mx_cmd_test(multix);
			break;
		case MX_CMD_ERR_0:
			mx_int_enqueue(multix, MX_IRQ_IEPS0, 0);
			break;
		case MX_CMD_ERR_6:
			mx_int_enqueue(multix, MX_IRQ_IEPS6, 0);
			break;
		case MX_CMD_ERR_7:
			mx_int_enqueue(multix, MX_IRQ_IEPS7, 0);
			break;
		case MX_CMD_ERR_8:
			mx_int_enqueue(multix, MX_IRQ_IEPS8, 0);
			break;
		case MX_CMD_ERR_C:
			mx_int_enqueue(multix, MX_IRQ_IEPSC, 0);
			break;
		case MX_CMD_ERR_D:
			mx_int_enqueue(multix, MX_IRQ_IEPSD, 0);
			break;
		case MX_CMD_ERR_E:
			mx_int_enqueue(multix, MX_IRQ_IEPSE, 0);
			break;
		case MX_CMD_ERR_F:
			mx_int_enqueue(multix, MX_IRQ_IEPSF, 0);
			break;
		default:
			break;
	}
}

// -----------------------------------------------------------------------
static void log_event(const char *text, struct mx_event *ev)
{
	char buf[1024];
	char *b = buf;
	b += sprintf(b, "(EV%04x) %s: %s", ev->id, text, mx_get_event_name(ev->type));
	if (ev->type == MX_EV_CMD) {
		b += sprintf(b, ", command: %s (%i)", mx_get_cmd_name(ev->cmd), ev->cmd);
		switch (ev->cmd) {
		case MX_CMD_STATUS: // z linią
		case MX_CMD_TRANSMIT:
		case MX_CMD_ATTACH:
		case MX_CMD_DETACH:
		case MX_CMD_ABORT:
			sprintf(b, ", line: %i, arg: 0x%04x", ev->log_n, ev->arg);
			break;
		case MX_CMD_SETCFG: // z arg
			break;
			sprintf(b, ", arg: 0x%04x", ev->arg);
		}
	}
	LOG(L_MX, "%s", buf);
}

// -----------------------------------------------------------------------
bool mx_init_dummy(struct mx *multix)
{
	bool quit = false;
	int timeout = MX_INIT_TIME_MSEC;

	LOG(L_MX, "Initialization delay: %i ms", timeout);

	while (!quit) {
		struct mx_event *ev = (struct mx_event *) elst_wait_pop(multix->eventq, timeout);
		if (!ev) {
			atom_store_release(&multix->state, MX_INITIALIZED);
			LOG(L_MX, "Multix is now initialized");
			mx_int_enqueue(multix, MX_IRQ_IWYZE, 0);
			break;
		} else {
			log_event("Got new event while still initializing", ev);
			if (ev->type == MX_EV_RESET) {
				// another reset, rinse and repeat
			} else if (ev->type == MX_EV_QUIT) {
				quit = true;
			} else {
				// no other events should appear at this stage
			}
			free(ev);
		}
	}
	return quit;
}

// -----------------------------------------------------------------------
static void * mx_event_loop(void *ptr)
{
	bool quit;
	struct mx *multix = (struct mx *) ptr;

	quit = mx_init_dummy(multix);

	while (!quit) {
		struct mx_event *ev = (struct mx_event *) elst_wait_pop(multix->eventq, 0);
		log_event("Processing event", ev);

		switch (ev->type) {
			case MX_EV_QUIT:
				quit = true;
				break;
			case MX_EV_RESET:
				mx_lines_deinit(multix);
				elst_clear(multix->eventq);
				mx_int_reset(multix);
				quit = mx_init_dummy(multix);
				break;
			case MX_EV_INT_PUSH:
				mx_int_push(multix);
				break;
			case MX_EV_CMD:
				mx_process_cmd_event(multix, ev);
			default:
				break;
		}
		free(ev);
	}

	LOG(L_MX, "Leaving event loop");
	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
static int mx_event(struct mx *multix, int type, int cmd, int log_n, uint16_t r_arg)
{
	static unsigned id;

	int state = atom_load_acquire(&multix->state);

	if (state == MX_QUIT) {
		LOG(L_MX, "Adding new event ignored: Multix is shutting down");
		return IO_EN;
	} else if ((state == MX_UNINITIALIZED) && (type != MX_EV_RESET) && (type != MX_EV_QUIT)) {
		LOG(L_MX, "Adding new event ignored: Multix is initializing and event is not RESET nor QUIT.");
		return IO_EN;
	}

	struct mx_event *ev = (struct mx_event *) malloc(sizeof(struct mx_event));
	if (!ev) return IO_EN;

	ev->type = type;
	ev->cmd = cmd;
	ev->log_n = log_n;
	ev->arg = r_arg;
	ev->id = id++;

	log_event("New event", ev);

	// type is also the priority
	if (elst_insert(multix->eventq, ev, type) < 0) {
		log_event("ERROR: Could not add event to the queue", ev);
		free(ev);
		return IO_EN;
	}

	return IO_OK;
}

// -----------------------------------------------------------------------
void mx_cmd_reset(void *ch)
{
	struct mx *multix = (struct mx *) ch;

	if (atom_load_acquire(&multix->state) == MX_QUIT) {
		LOG(L_MX, "Reset request ignored, Multix is shutting down");
		return;
	}

	atom_store_release(&multix->state, MX_UNINITIALIZED); // as early as possible
	mx_event(multix, MX_EV_RESET, 0, 0, 0);
	// actual reset is done in event processor thread
}

// -----------------------------------------------------------------------
int mx_cmd(void *ch, int dir, uint16_t n_arg, uint16_t *r_arg)
{
	struct mx *multix = (struct mx *) ch;

	const unsigned cmd      = ((n_arg & 0b1110000000000000) >> 13) | ((dir&1) << 3);
	const unsigned chan_cmd =  (n_arg & 0b0001100000000000) >> 11;
	const unsigned log_n    =  (n_arg & 0b0001111111100000) >> 5;

	if (cmd == MX_CMD_CHAN) {
		LOG(L_MX, "Channel command: %s (%i)", mx_get_chan_cmd_name(chan_cmd), chan_cmd);
		switch (chan_cmd) {
			case MX_CHAN_CMD_INTSPEC:
				*r_arg = mx_int_get_spec(multix);
				mx_event(multix, MX_EV_INT_PUSH, 0, 0, 0);
				return IO_OK;
			case MX_CHAN_CMD_EXISTS:
				return IO_OK;
			case MX_CHAN_CMD_RESET:
				mx_cmd_reset(multix);
				return IO_OK;
			default: // MX_CHAN_CMD_INVALID
				// fallthrough to line or general event processing
				break;
		}
	} else {
		LOG(L_MX, "General or line %i command: %s (%i)", log_n, mx_get_cmd_name(cmd), cmd);
	}

	return mx_event(multix, MX_EV_CMD, cmd, log_n, *r_arg);
}

// -----------------------------------------------------------------------
const struct chan_drv mx_chan_driver = {
	.name = "multix",
	.create = mx_create,
	.shutdown = mx_shutdown,
	.reset = mx_cmd_reset,
	.cmd = mx_cmd
};

// vim: tabstop=4 shiftwidth=4 autoindent
