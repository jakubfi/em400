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

#define _GNU_SOURCE

#include <stdlib.h>
#include <pthread.h>

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

enum mx_condition { MX_UNINITIALIZED, MX_INITIALIZED, MX_CONFIGURED, MX_QUIT };

// Real multix boots up in probably just under a second
// (~500ms for ROM/RAM check + ~185ms for RAM cleanup).
// Here we need just a reasonable delay - big enough for
// OS scheduler to switch threads between MULTIX and CPU,
// so we don't finish MULTIX' job before switching back to CPU thread.
#define MX_INIT_TIME_MSEC 150

typedef int (*mx_cmd_fun)(struct mx *multix, int log_n, uint16_t arg);

static void * mx_evproc(void *ptr);
static int mx_event(struct mx *multix, int type, int cmd, int log_n, uint16_t r_arg);
int mx_int_enqueue(struct mx *multix, int intr, int line);
void mx_reset(void *ch);

// -----------------------------------------------------------------------
void * mx_create(int num, struct cfg_unit *units)
{
	LOG(L_MX, "Creating new MULTIX");

	// --- create multix itself (everything needs it)

	struct mx *multix = calloc(1, sizeof(struct mx));
	if (!multix) {
		LOGERR("Memory allocation error.");
		goto cleanup;
	}
	// initialize multix structure
	multix->chnum = num;
	atom_store_release(&multix->state, MX_UNINITIALIZED);
	for (int i=0 ; i<MX_LINE_CNT ; i++) {
		struct mx_line *pline = multix->plines + i;
		pline->phy_n = i;
		pline->multix = multix;
		pline->protoq = elst_create(1024);
		pline->statusq = elst_create(1024);
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
	multix->intq = elst_create(1024);
	if (!multix->intq) {
		LOGERR("Failed to create interrupt queue.");
		goto cleanup;
	}

	// --- create devices (event system need them)

	LOG(L_MX, "Initializing devices");
	struct cfg_unit *dev_cfg = units;
	while (dev_cfg) {
		struct mx_line *line = multix->plines + dev_cfg->num;
		int res = dev_make(dev_cfg, &line->dev, &line->dev_data);
		if (res != E_OK) {
			LOGERR("Failed to create MULTIX device: %s", dev_cfg->name);
			goto cleanup;
		}
		dev_cfg = dev_cfg->next;
	}

	// --- create event system (MERA-400 interface needs it)

	multix->eventq = elst_create(1024);
	if (!multix->eventq) {
		LOGERR("Failed to create event queue.");
		goto cleanup;
	}
	if (pthread_create(&multix->ev_thread, NULL, mx_evproc, multix)) {
		LOGERR("Failed to spawn event processor thread.");
		goto cleanup;
	}

	pthread_setname_np(multix->ev_thread, "multixev");

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
			union mx_event *ev = malloc(sizeof(union mx_event));
			if (ev) {
				ev->d.type = MX_EV_QUIT;
				if (elst_insert(lline->protoq, ev, MX_EV_QUIT) > 0) {
					pthread_join(lline->proto_th, NULL);
				} else {
					LOG(L_MX, "Failed to send QUIT event to %s protocol queue on line %i, terminating event thread", lline->proto->name, lline->log_n );
					pthread_cancel(lline->proto_th);
				}
			}
			// quit the status thread
			union mx_event *ev2 = malloc(sizeof(union mx_event));
			if (ev2) {
				ev2->d.type = MX_EV_QUIT;
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
		pthread_mutex_destroy(&pline->status_mutex);
	}
	free(multix);
	LOG(L_MX, "Shutdown complete");
}

// -----------------------------------------------------------------------
int mx_mem_mget(struct mx *multix, int nb, uint16_t addr, uint16_t *data, int len)
{
	if (atom_load_acquire(&multix->state) != MX_UNINITIALIZED) {
		if (io_mem_mget(nb, addr, data, len) != len) {
			return -1;
		}
	} else {
		LOG(L_MX, "LOST memory read due to multix initializing");
	}
	return 0;
}

// -----------------------------------------------------------------------
int mx_mem_mput(struct mx *multix, int nb, uint16_t addr, uint16_t *data, int len)
{
	if (atom_load_acquire(&multix->state) != MX_UNINITIALIZED) {
		if (io_mem_mput(nb, addr, data, len) != len) {
			return -1;
		}
	} else {
		LOG(L_MX, "LOST memory write due to multix initializing");
	}
	return 0;
}

// -----------------------------------------------------------------------
static void mx_int_set(struct mx *multix)
{
	if (atom_load_acquire(&multix->state) != MX_UNINITIALIZED) {
		io_int_set(multix->chnum);
	} else {
		LOG(L_MX, "LOST interrupt due to multix initializing");
	}
}

// -----------------------------------------------------------------------
static void mx_int_push(struct mx *multix)
{
	int send = 0;

	pthread_mutex_lock(&multix->int_mutex);
	if (multix->intspec == MX_IRQ_INIEA) {
		uint16_t *i = elst_nlock_pop(multix->intq);
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
	uint16_t *i = malloc(sizeof(uint16_t));
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
	snprintf(thname, 15, "mxline%02i", pline->log_n);
	if (pthread_create(&pline->proto_th, NULL, mx_line_thread, pline)) {
		return MX_SC_E_NOMEM;
	}
	pthread_setname_np(pline->proto_th, thname);

	snprintf(thname, 15, "mxstat%02i", pline->log_n);
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
	uint16_t data[MX_LINE_CNT+4*MX_LINE_CNT];

	int ret_int;
	uint16_t ret_err = 0;

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

	unsigned phy_desc_count = (data[0] & 0b1111111100000000) >> 8;
	unsigned log_count      = (data[0] & 0b0000000011111111);

	LOG(L_MX, "Configuration for MULTIX on channel %i has %i physical line descriptors, %i logical lines", multix->chnum, phy_desc_count, log_count);

	// read line descriptions
	int read_size = phy_desc_count + 4*log_count;
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
	int cur_line = 0;
	for (int i=0 ; i<phy_desc_count ; i++) {
		unsigned count = (data[i] & 0b11111) + 1;
		LOG(L_MX, "%i Physical line(-s) %i..%i:", count, cur_line, cur_line+count-1);
		for (int j=0 ; j<count ; j++, cur_line++) {
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
	int tape_formatters = 0;
	for (int i=0 ; i<MX_LINE_CNT ; i+=4) {
		// there can be only one tape formatter (4 lines)
		if ((multix->plines[i].type == MX_PHY_MTAPE) && (++tape_formatters > 1)) {
			CFGERR(MX_SC_E_PHY_INCOMPLETE, i);
			goto fail;
		}
		// MULTIX lines are physically organized in 4-line groups and configuration needs to reflect this
		for (int j=1 ; j<=3 ; j++) {
			if (multix->plines[i+j].type != multix->plines[i].type) {
				CFGERR(MX_SC_E_PHY_INCOMPLETE, i+j);
				goto fail;
			}
		}
	}

	// configure logical lines
	for (int i=0 ; i<log_count ; i++) {
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
	// TODO: I think real multix would self-reset after test finish...
	mx_int_enqueue(multix, MX_IRQ_IWYTE, 0);
	return 0;
}

// -----------------------------------------------------------------------
static int mx_cmd_requeue(struct mx *multix)
{
	pthread_mutex_lock(&multix->int_mutex);
	if (multix->intspec != MX_IRQ_INIEA) {
		uint16_t *i = malloc(sizeof(uint16_t));
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
static int mx_conf_check(struct mx *multix, struct mx_line *lline, union mx_event *ev)
{
	// is multix configured?
	if (atom_load_acquire(&multix->state) == MX_UNINITIALIZED) {
		LOG(L_MX, "EV%04x: Rejecting command, MULTIX not initialized", ev->d.id);
		return -1;
	}

	// is the line configured?
	if (!lline) {
		LOG(L_MX, "EV%04x: Rejecting command, line %i not configured", ev->d.log_n, ev->d.id);
		return -1;
	}

	return 0;
}

// -----------------------------------------------------------------------
static int mx_cmd_dispatch(struct mx *multix, struct mx_line *lline, union mx_event *ev)
{
	// is multix and line configured?
	if (mx_conf_check(multix, lline, ev)) {
		mx_int_enqueue(multix, mx_irq_noline(ev->d.cmd), ev->d.log_n);
		return 1;
	}

	const struct mx_cmd *cmd = lline->proto->cmd + ev->d.cmd;

	if (!cmd->run && (ev->d.cmd != MX_CMD_STATUS)) { // NOTE: 'status' command is not a protocol command
		LOG(L_MX, "EV%04x: Rejecting command: no protocol function to handle command %s for protocol %s in line %i", ev->d.id, mx_get_cmd_name(ev->d.cmd), lline->proto->name, lline->log_n);
		mx_int_enqueue(lline->multix, mx_irq_reject(ev->d.cmd), ev->d.log_n);
		return 1;
	}

	// can line process the command?
	pthread_mutex_lock(&lline->status_mutex);
	if (mx_line_cmd_allowed(lline, ev->d.cmd)) {
		log_line_status("Rejecting command, line status does not allow execution", ev->d.log_n, lline->status, ev->d.id);
		mx_int_enqueue(lline->multix, mx_irq_reject(ev->d.cmd), ev->d.log_n);
		pthread_mutex_unlock(&lline->status_mutex);
		return 1;
	}
	// update line status
	lline->status |= mx_cmd_state(ev->d.cmd);
	pthread_mutex_unlock(&lline->status_mutex);

	// process asynchronously in the protocol thread
	LOG(L_MX, "EV%04x: Enqueue command %s for %s in line %i", ev->d.id, mx_get_cmd_name(ev->d.cmd), lline->proto->name, lline->log_n);
	if (ev->d.cmd == MX_CMD_STATUS) {
		elst_append(lline->statusq, ev);
	} else {
		elst_append(lline->protoq, ev);
	}

	return 0; // don't delete the event
}

// -----------------------------------------------------------------------
static void * mx_evproc(void *ptr)
{
	int quit = 0;
	int timeout = MX_INIT_TIME_MSEC;
	struct mx *multix = ptr;

	LOG(L_MX, "Entering event loop");

	while (!quit) {
		if (timeout > 0) {
			LOG(L_MX, "Initialization delay: %i ms", timeout);
		} else {
			LOG(L_MX, "Event processor waiting for event");
		}
		union mx_event *ev = elst_wait_pop(multix->eventq, timeout);

		int ev_free = 1;

		if (!ev) { // initialization finished, maybe?
			if (timeout) { // loop was indeed waiting for the initialization timeout
				timeout = 0;
				atom_store_release(&multix->state, MX_INITIALIZED);
				LOG(L_MX, "Multix is now initialized");
				mx_int_enqueue(multix, MX_IRQ_IWYZE, 0);
			} else {
				LOG(L_MX, "ERROR: Received unexpected NULL event");
			}
		} else { // regular event processing
			LOG(L_MX, "EV%04x: Received event: %s", ev->d.id, mx_get_event_name(ev->d.type));
			switch (ev->d.type) {
				case MX_EV_QUIT:
					// not ignored when resetting MULTIX
					// called by mx_shutdown()
					// here we just need to exit the event processor loop
					// mx_shutdown() will do all the cleanup
					quit = 1;
					break;
				case MX_EV_RESET:
					// not ignored when resetting MULTIX - just do it all over again
					// reset makes multix uninitialized (as before setcfg)
					mx_lines_deinit(multix);
					elst_clear(multix->eventq);
					mx_int_reset(multix);
					// set init wait timeout and let the event loop run again
					// (it will timeout on elst_wait_pop() and continue with sending interrupt)
					timeout = MX_INIT_TIME_MSEC;
					break;
				case MX_EV_INT_PUSH:
					if (timeout) continue; // ignore INT_PUSH when resetting MULTIX
					mx_int_push(multix);
					break;
				case MX_EV_CMD:
					if (timeout) continue; // ignore commands when restting MULTIX
					LOG(L_MX, "EV%04x: Received command: %s", ev->d.id, mx_get_cmd_name(ev->d.cmd));
					struct mx_line *lline = multix->llines[ev->d.log_n];
					switch (ev->d.cmd) {
						case MX_CMD_REQUEUE:
							mx_cmd_requeue(multix);
							break;
						case MX_CMD_STATUS:
						case MX_CMD_TRANSMIT:
						case MX_CMD_ATTACH:
						case MX_CMD_DETACH:
						case MX_CMD_ABORT:
							ev_free = mx_cmd_dispatch(multix, lline, ev);
							break;
						case MX_CMD_SETCFG:
							mx_cmd_setcfg(multix, ev->d.arg);
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
					break;
				default:
					break;
			}
		}
		if (ev_free) {
			free(ev);
		}
	}

	LOG(L_MX, "Left event loop");

	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
static int mx_event(struct mx *multix, int type, int cmd, int log_n, uint16_t r_arg)
{
	static unsigned id;

	if (atom_load_acquire(&multix->state) == MX_QUIT) {
		LOG(L_MX, "Create new event ignored, Multix is shutting down");
		return IO_EN;
	}

	union mx_event *ev = malloc(sizeof(union mx_event));
	if (!ev) return IO_EN;

	ev->d.type = type;
	ev->d.cmd = cmd;
	ev->d.log_n = log_n;
	ev->d.arg = r_arg;
	ev->d.id = id++;

	LOG(L_MX, "EV%04x: Created new event %s, command: %i (%s), line: %i, arg: 0x%04x ",
		ev->d.id,
		mx_get_event_name(ev->d.type),
		ev->d.cmd,
		(ev->d.type == MX_EV_CMD) ? mx_get_cmd_name(ev->d.cmd) : "",
		ev->d.log_n,
		ev->d.arg
	);

	// type is also the priority
	if (elst_insert(multix->eventq, ev, type) < 0) {
		return IO_EN;
	}
	return IO_OK;
}

// -----------------------------------------------------------------------
void mx_reset(void *ch)
{
	struct mx *multix = (struct mx *) ch;

	if (atom_load_acquire(&multix->state) == MX_QUIT) {
		LOG(L_MX, "Received reset request ignored, Multix is shutting down");
		return;
	}

	LOG(L_MX, "Received reset request");
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

	if (cmd == MX_CMD_CHAN) { // channel commands
		LOG(L_MX, "Received channel command: %s (%i)", mx_get_chan_cmd_name(chan_cmd), chan_cmd);
		switch (chan_cmd) {
			case MX_CHAN_CMD_INTSPEC:
				*r_arg = mx_int_get_spec(multix);
				mx_event(multix, MX_EV_INT_PUSH, 0, 0, 0);
				return IO_OK; // always OK
			case MX_CHAN_CMD_EXISTS:
				return IO_OK; // always OK
			case MX_CHAN_CMD_RESET:
				mx_reset(multix);
				return IO_OK; // always OK, although there is no actual response for reset on the system bus
		}
	}

	if (atom_load_acquire(&multix->state) == MX_UNINITIALIZED) {
		// ignore commands (respond with EN) when multix is initializing
		LOG(L_MX, "EN for received general or line %i command: %s (%i)", log_n, mx_get_cmd_name(cmd), cmd);
		return IO_EN;
	} else {
		LOG(L_MX, "Received general or line %i command: %s (%i)", log_n, mx_get_cmd_name(cmd), cmd);
		return mx_event(multix, MX_EV_CMD, cmd, log_n, *r_arg);
	}
}

// -----------------------------------------------------------------------
const struct chan_drv mx_chan_driver = {
	.name = "multix",
	.create = mx_create,
	.shutdown = mx_shutdown,
	.reset = mx_reset,
	.cmd = mx_cmd
};

// vim: tabstop=4 shiftwidth=4 autoindent
