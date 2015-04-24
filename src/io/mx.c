//  Copyright (c) 2012-2015 Jakub Filipowicz <jakubf@gmail.com>
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

#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "errors.h"
#include "log.h"
#include "mem/mem.h"
#include "io/chan.h"
#include "io/mx.h"
#include "io/mx_cmds.h"
#include "io/mx_ev.h"
#include "io/mx_irq.h"

// Real multix boots up in probably just under a second
// ~500ms (for ROM/RAM check) + ~185ms (for RAM cleanup).
// Here we need just a reasonable delay - big enough for
// OS scheduler to switch threads between MULTIX and CPU,
// so we don't finish MULTIX' job before switching back to CPU thread.
#define MX_RESET_WAIT_MSEC 150

// internel MULTIX' timer step is 500ms
#define MX_TIMER_STEP_MS 500

enum mx_state {
	MX_STATE_RUN, MX_STATE_QUIT, MX_STATE_RESET
};

const char *mx_state_names[] = {
	"RUN", "QUIT", "RESET"
};

// -----------------------------------------------------------------------
// ---- channel driver interface -----------------------------------------
// -----------------------------------------------------------------------

void * mx_create(int num, struct cfg_unit *units);
void mx_shutdown(void *ch);
void mx_reset(void *ch);
int mx_cmd(void *ch, int dir, uint16_t n_arg, uint16_t *r_arg);

struct chan_drv mx_chan_driver = {
	.name = "multix",
	.create = mx_create,
	.shutdown = mx_shutdown,
	.reset = mx_reset,
	.cmd = mx_cmd
};

static void * mx_main(void *ptr);

// -----------------------------------------------------------------------
void * mx_create(int num, struct cfg_unit *units)
{
	struct mx *multix = calloc(1, sizeof(struct mx));
	if (!multix) {
		gerr = E_ALLOC;
		goto cleanup;
	}

	multix->num = num;

	LOG_SET_ID(multix, "MX %i", multix->num);

	LOGID(L_MX, 1, multix, "Creating new MULTIX");

	// state initialization
	multix->state = MX_STATE_RESET;
	multix->reset_ack = 0;
	if (pthread_mutex_init(&multix->state_mutex, NULL)) {
		goto cleanup;
	}
	if (pthread_cond_init(&multix->state_cond, NULL)) {
		goto cleanup;
	}
	if (pthread_mutex_init(&multix->reset_ack_mutex, NULL)) {
		goto cleanup;
	}
	if (pthread_cond_init(&multix->reset_ack_cond, NULL)) {
		goto cleanup;
	}

	// create event queue
	multix->evq = mx_evq_create(-1);
	if (!multix->evq) {
		gerr = E_MX_EVQ;
		goto cleanup;
	}
	LOG_SET_ID(multix->evq, "%s EV", LOG_GET_ID(multix));

	// create interrupt system
	multix->irqq = mx_irqq_create(multix->num, MX_INTRQ_LEN);
	if (!multix->irqq) {
		gerr = E_MX_IRQQ;
		goto cleanup;
	}
	LOG_SET_ID(multix->irqq, "%s IRQ", LOG_GET_ID(multix));

	// create timer
	multix->timer = mx_timer_init(MX_TIMER_STEP_MS, multix->evq);
	if (!multix->irqq) {
		gerr = E_MX_TIMER;
		goto cleanup;
	}
	LOG_SET_ID(multix->timer, "%s TIMER", LOG_GET_ID(multix));

	// initialize main thread
	if (pthread_create(&multix->main_th, NULL, mx_main, multix)) {
		gerr = E_THREAD;
		goto cleanup;
	}

	return multix;

cleanup:
	mx_shutdown(multix);
	return NULL;
}

// -----------------------------------------------------------------------
void mx_shutdown(void *ch)
{
	if (!ch) return;

	struct mx *multix = (struct mx *) ch;

	LOGID(L_MX, 1, multix, "Shutting down");

	// notify main thread to quit
	pthread_mutex_lock(&multix->state_mutex);
	multix->state = MX_STATE_QUIT;
	pthread_cond_signal(&multix->state_cond);
	pthread_mutex_unlock(&multix->state_mutex);

	// stop accepting events
	mx_evq_disable(multix->evq);

	// wait for the main thread to quit
	LOGID(L_MX, 3, multix, "Waiting for main thread to join");
	pthread_join(multix->main_th, NULL);

	// At this point we're sure that MULTIX thread is down

	// shutdown timer
	mx_timer_shutdown(multix->timer);

	// destroy interrupt system
	mx_irqq_destroy(multix->irqq);

	// destroy event queue
	mx_evq_destroy(multix->evq);

	// free resources
	pthread_mutex_destroy(&multix->state_mutex);
	pthread_cond_destroy(&multix->state_cond);
	pthread_mutex_destroy(&multix->reset_ack_mutex);
	pthread_cond_destroy(&multix->reset_ack_cond);

	LOGID(L_MX, 3, multix, "Shutdown complete");

	free(multix);
}

// -----------------------------------------------------------------------
void mx_reset(void *ch)
{
	struct mx *multix = (struct mx *) ch;

	LOGID(L_MX, 2, multix, "Initiating reset");

	// set multix state to "reset"
	pthread_mutex_lock(&multix->state_mutex);
	if (multix->state != MX_STATE_QUIT) {
		multix->state = MX_STATE_RESET;
		pthread_cond_signal(&multix->state_cond);
	}
	pthread_mutex_unlock(&multix->state_mutex);

	// disable event queue
	// waiting evq_dequeue()s will return and further commands will fail
	mx_evq_disable(multix->evq);

	// wait for reset to really kick in
	pthread_mutex_lock(&multix->reset_ack_mutex);
	while (!multix->reset_ack) {
		LOGID(L_MX, 3, multix, "Waiting for reset to kick in");
		pthread_cond_wait(&multix->reset_ack_cond, &multix->reset_ack_mutex);
	}
	pthread_mutex_unlock(&multix->reset_ack_mutex);
	LOGID(L_MX, 2, multix, "Reset initiated");
}

// -----------------------------------------------------------------------
int mx_cmd(void *ch, int dir, uint16_t n_arg, uint16_t *r_arg)
{
	struct mx *multix = (struct mx *) ch;
	struct mx_ev *ev;

	unsigned cmd		= ((n_arg & 0b1110000000000000) >> 13) | ((dir&1) << 3);
	unsigned chan_cmd	=  (n_arg & 0b0001100000000000) >> 11;
	unsigned log_n		=  (n_arg & 0b0001111111100000) >> 5;

	// channel commands
	if (cmd == MX_CMD_CHAN) {
		LOGID(L_MX, 2, multix, "Incomming channel command %i: %s", chan_cmd, mx_get_chan_cmd_name(chan_cmd));

		switch (chan_cmd) {
			case MX_CHAN_CMD_INTSPEC:
				// get intspec (always there, even if invalid)
				*r_arg = mx_irqq_get_intspec(multix->irqq);
				// notify main thread that IRQ has been received byu CPU
				ev = mx_ev_simple(MX_EV_INT_RECVD);
				if (mx_evq_enqueue(multix->evq, ev, MX_EVQ_F_WAIT) <= 0) {
					mx_ev_delete(ev);
				}
				return IO_OK;
			case MX_CHAN_CMD_EXISTS:
				// 'exists' does nothing, just returns OK
				return IO_OK;
			case MX_CHAN_CMD_RESET:
				// initiate reset and return OK (actual reset is done in main thread)
				mx_reset(multix);
				return IO_OK;
			default:
				// handle other commands (only MX_CMD_INVALID in fact) as illegal in main thread
				ev = mx_ev_cmd(cmd, 0, 0);
				if (mx_evq_enqueue(multix->evq, ev, MX_EVQ_F_TRY) <= 0) {
					LOGID(L_MX, 2, multix, "ENGAGED");
					mx_ev_delete(ev);
					return IO_EN;
				} else {
					return IO_OK;
				}
		}
	// handle general and line commands in main thread
	} else {
		LOGID(L_MX, 2, multix, "Incomming general/line command %i for line %i: %s", cmd, log_n, mx_get_cmd_name(cmd));
		ev = mx_ev_cmd(cmd, log_n, *r_arg);
		if (mx_evq_enqueue(multix->evq, ev, MX_EVQ_F_TRY) <= 0) {
			LOGID(L_MX, 2, multix, "ENGAGED");
			mx_ev_delete(ev);
			return IO_EN;
		} else {
			return IO_OK;
		}
	}
}

// -----------------------------------------------------------------------
// ---- MULTIX -----------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
static void mx_deconfigure(struct mx *multix)
{
	mx_timer_off(multix->timer);
	multix->conf_set = 0;
	multix->log_line_count = 0;

	for (int i=0 ; i<MX_LINE_MAX ; i++) {
		struct mx_line *line = multix->lines + i;
		mx_line_deconfigure(line);
		multix->log_lines[i] = NULL;
		LOG_SET_ID(line, "%s:? (p?)", LOG_GET_ID(multix));
	}
}

// -----------------------------------------------------------------------
static int mx_init(struct mx *multix)
{
	LOGID(L_MX, 2, multix, "Entering initialization loop");

	int ret;

	mx_deconfigure(multix);

	pthread_mutex_lock(&multix->state_mutex);

	// notify CPU thread that reset has been initiated
	// (i.e. MULTIX won't send anything to CPU from now on)
	pthread_mutex_lock(&multix->reset_ack_mutex);
	multix->reset_ack = 1;
	pthread_cond_signal(&multix->reset_ack_cond);
	pthread_mutex_unlock(&multix->reset_ack_mutex);

	// loop while state is RESET
	while (multix->state == MX_STATE_RESET) {
		// we set state to RUN here - if another reset is initiated in the cpu thread,
		// and state changes to RESET during wait on state_cond and we'll loop again.
		// If state changes to QUIT, we exit the loop and handle QUIT
		multix->state = MX_STATE_RUN;

		// set abstime to MX_RESET_WAIT_MSEC in the future, wall clock
		struct timespec abstime;
		clock_gettime(CLOCK_REALTIME, &abstime);
		long new_nsec = abstime.tv_nsec + MX_RESET_WAIT_MSEC * 1000000L;
		abstime.tv_sec += new_nsec / 1000000000L;
		abstime.tv_nsec = new_nsec % 1000000000L;

		LOGID(L_MX, 3, multix, "Starting initialization delay: %i ms", MX_RESET_WAIT_MSEC);

		// wait on condition for MX_RESET_WAIT_MSEC ms
		int res = pthread_cond_timedwait(&multix->state_cond, &multix->state_mutex, &abstime);
		LOGID(L_MX, 3, multix, "Initialization wait %s (\"%s\"), MULTIX state is now: %s",
			(res == 0) ? "interrupted" : "finished",
			strerror(res),
			mx_state_names[multix->state]
		);
	}

	if (multix->state == MX_STATE_QUIT) {
		LOGID(L_MX, 3, multix, "Quit received during initialization");
		ret = -1;
	} else {
		mx_evq_clear(multix->evq);
		mx_irqq_clear(multix->irqq);
		mx_evq_enable(multix->evq);
		mx_irqq_enqueue(multix->irqq, MX_IRQ_IWYZE, 0);

		// clear reset ack flag
		pthread_mutex_lock(&multix->reset_ack_mutex);
		multix->reset_ack = 0;
		pthread_cond_signal(&multix->reset_ack_cond);
		pthread_mutex_unlock(&multix->reset_ack_mutex);

		LOGID(L_MX, 2, multix, "Initialization done");
		ret = 0;
	}

	pthread_mutex_unlock(&multix->state_mutex);

	return ret;
}

// -----------------------------------------------------------------------
static void mx_test(struct mx *multix)
{
	// As we're not able to run any real 8085 code, TEST command
	// won't work. Best we can do is to pretend that test is done
	// and let the test wrapper on CPU side worry about the (non-)results.
	mx_evq_disable(multix->evq);
	mx_deconfigure(multix);
	mx_evq_clear(multix->evq);
	mx_irqq_clear(multix->irqq);
	mx_evq_enable(multix->evq);
	mx_irqq_enqueue(multix->irqq, MX_IRQ_IWYTE, 0);
}

// -----------------------------------------------------------------------
static void mx_setcfg_fin(struct mx *multix, int irq, uint16_t addr, unsigned result, unsigned line)
{
	// configuration was bad
	if (irq == MX_IRQ_INKON) {
		if (result >= MX_SC_E_PROTO_MISSING) {
			LOGID(L_MX, 1, multix, "Configuration not set. Error for logical line %i: %s", line, mx_line_sc_err_name(result));
		} else if (result >= MX_SC_E_DEVTYPE) {
			LOGID(L_MX, 1, multix, "Configuration not set. Error for physical line %i: %s", line, mx_line_sc_err_name(result));
		} else {
			LOGID(L_MX, 1, multix, "Configuration not set: %s", mx_line_sc_err_name(result));
		}
		// store command result
		if (!mem_put(0, addr, (result<<8) | line)) {
			irq = MX_IRQ_INKOT;
		}
	// configuration was OK
	} else if (irq == MX_IRQ_IUKON) {
		LOGID(L_MX, 1, multix, "Configuration successfully set");
	}

	// couldn't store the result
	if (irq == MX_IRQ_INKOT) {
		LOGID(L_MX, 1, multix, "Configuration not set: memory access error");
	}

	mx_irqq_enqueue(multix->irqq, irq, 0);
}

// -----------------------------------------------------------------------
static void mx_setcfg(struct mx *multix, uint16_t addr)
{
	int res;

	uint16_t data[MX_LINE_MAX+4*MX_LINE_MAX];
	uint16_t retf_addr = addr + 1;

	// check if configuration is already set
	if (multix->conf_set) {
		mx_setcfg_fin(multix, MX_IRQ_INKON, retf_addr, MX_SC_E_CONFSET, 0);
		return;
	}

	mx_deconfigure(multix);

	// read configuration header
	if (!mem_get(0, addr, data)) {
		mx_setcfg_fin(multix, MX_IRQ_INKOT, retf_addr, 0, 0);
		return;
	}

	unsigned phy_desc_count	= (*data & 0b1111111100000000) >> 8;
	unsigned log_count		= (*data & 0b0000000011111111);

	LOGID(L_MX, 3, multix, "Configuring: %i physical line descriptors, %i logical lines", multix->num, phy_desc_count, log_count);

	// read line descriptions
	if (!mem_mget(0, addr+2, data, phy_desc_count + 4*log_count)) {
		mx_setcfg_fin(multix, MX_IRQ_INKOT, retf_addr, 0, 0);
		return;
	}

	// check if number of phy line descriptors and log line counts are OK
	if ((phy_desc_count <= 0) || (phy_desc_count > MX_LINE_MAX) || (log_count <= 0) || (log_count > MX_LINE_MAX)) {
		mx_setcfg_fin(multix, MX_IRQ_INKON, retf_addr, MX_SC_E_NUMLINES, 0);
		return;
	}

	// configure physical lines
	int cur_line = 0;
	for (int i=0 ; i<phy_desc_count ; i++) {
		unsigned count		= (data[i] & 0b0000000000011111) + 1;
		LOGID(L_MX, 3, multix, "  %i Physical line(-s) %i..%i:", count, cur_line, cur_line+count-1);
		for (int j=0 ; j<count ; j++, cur_line++) {
			LOG_SET_ID(multix->lines+cur_line, "%s:? (p%i)", LOG_GET_ID(multix), cur_line);
			if (cur_line >= MX_LINE_MAX) {
				mx_setcfg_fin(multix, MX_IRQ_INKON, retf_addr, MX_SC_E_NUMLINES, 0);
				return;
			}
			res = mx_line_conf_phy(multix->lines+cur_line, data[i]);
			if (res != MX_SC_E_OK) {
				mx_setcfg_fin(multix, MX_IRQ_INKON, retf_addr, res, cur_line);
				return;
			}
		}
	}

	// check the completness of physical lines configuration
	int tape_formatters = 0;
	for (int i=0 ; i<MX_LINE_MAX ; i+=4) {
		// there can be only one tape formatter (4 lines)
		if ((multix->lines[i].type == MX_PHY_MTAPE) && (++tape_formatters > 1)) {
			mx_setcfg_fin(multix, MX_IRQ_INKON, retf_addr, MX_SC_E_PHY_INCOMPLETE, i);
			return;
		}
		// MULTIX lines are physically organized in 4-line groups and configuration needs to reflect this
		for (int j=1 ; j<=3 ; j++) {
			if (multix->lines[i+j].type != multix->lines[i].type) {
				mx_setcfg_fin(multix, MX_IRQ_INKON, retf_addr, MX_SC_E_PHY_INCOMPLETE, i+j);
				return;
			}
		}
	}

	// configure logical lines
	for (int i=0 ; i<log_count ; i++) {
		uint16_t *log_data = data+phy_desc_count+(i*4);
		unsigned phy_num = log_data[0] & 0b0000000000011111;
		struct mx_line *line = multix->lines + phy_num;

		LOG_SET_ID(line, "%s:%i (p%i)", LOG_GET_ID(multix), i, phy_num);

		res = mx_line_conf_log(line, log_data);
		if (res != MX_SC_E_OK) {
			mx_setcfg_fin(multix, MX_IRQ_INKON, retf_addr, res, i);
			return;
		}

		multix->log_lines[i] = line;
	}

	multix->log_line_count = log_count;
	multix->conf_set = 1;

	// start timer
	mx_timer_on(multix->timer);

	mx_setcfg_fin(multix, MX_IRQ_IUKON, retf_addr, 0, 0);
}

// -----------------------------------------------------------------------
// Given the command, we need to know what task to report or
// what IRQ to send when configuration is missing or task cannot
// be accepted
struct mx_cmd_route {
	int task_num;
	int irq_reject;
	int irq_no_line;
} mx_cmd_routing[] = {
// command              task                irq_reject
/* MX_CMD_ERR_0   */  { MX_TASK_UNKNOWN,	MX_IRQ_IEPS0,	-1 },
/* MX_CMD_TEST    */  { MX_TASK_MISPLCD,	-1,				-1 },
/* MX_CMD_ATTACH  */  { MX_TASK_ATTACH,		MX_IRQ_INDOL,	MX_IRQ_INKDO },
/* MX_CMD_STATUS  */  { MX_TASK_STATUS,		MX_IRQ_INSTR,	MX_IRQ_INKST },
/* MX_CMD_TRANSMIT*/  { MX_TASK_TRANSMIT,	MX_IRQ_INTRA,	MX_IRQ_INKTR },
/* MX_CMD_SETCFG  */  { MX_TASK_MISPLCD,	-1,				-1 },
/* MX_CMD_ERR_6   */  { MX_TASK_UNKNOWN,	MX_IRQ_IEPS6,	-1 },
/* MX_CMD_ERR_7   */  { MX_TASK_UNKNOWN,	MX_IRQ_IEPS7,	-1 },
/* MX_CMD_ERR_8   */  { MX_TASK_UNKNOWN,	MX_IRQ_IEPS8,	-1 },
/* MX_CMD_INTRQ   */  { MX_TASK_MISPLCD,	-1,				-1 },
/* MX_CMD_DETACH  */  { MX_TASK_DETACH,		MX_IRQ_INODL,	MX_IRQ_INKOD },
/* MX_CMD_ABORT   */  { MX_TASK_ABORT,		MX_IRQ_INABT,	MX_IRQ_INKAB },
/* MX_CMD_ERR_C   */  { MX_TASK_UNKNOWN,	MX_IRQ_IEPSC,	-1 },
/* MX_CMD_ERR_D   */  { MX_TASK_UNKNOWN,	MX_IRQ_IEPSD,	-1 },
/* MX_CMD_ERR_E   */  { MX_TASK_UNKNOWN,	MX_IRQ_IEPSE,	-1 },
/* MX_CMD_ERR_F   */  { MX_TASK_UNKNOWN,	MX_IRQ_IEPSF,	-1 }
};

// -----------------------------------------------------------------------
static void mx_task_router(struct mx *multix, unsigned cmd, unsigned line_num, uint16_t arg)
{
	// commands that shouldn't happen at all
	if (cmd > MX_CMD_ERR_F) {
		LOGID(L_MX, 1, multix, "EMULATION ERROR: Unknown general/line command: %i (ignored)", cmd);
		return;
	}

	struct mx_line *line = multix->log_lines[line_num];
	struct mx_cmd_route *route = mx_cmd_routing + cmd;
	struct mx_task *task = line->task + route->task_num;

	// commands that are possible, but unknown for MULTIX
	if (route->task_num == MX_TASK_UNKNOWN) {
		LOGID(L_MX, 1, multix, "Unknown MULTIX command %i: %s", cmd, mx_get_cmd_name(cmd));
		mx_irqq_enqueue(multix->irqq, route->irq_reject, 0);

	// correct commands that are not suppose to be processed here
	} else if (route->task_num == MX_TASK_MISPLCD) {
		LOGID(L_MX, 1, multix, "EMULATION ERROR: bad command %i passed in the event: %s", cmd, mx_get_cmd_name(cmd));

	// proper commands
	} else {
		if (!multix->conf_set || !line) {
			LOGID(L_MX, 1, multix, "Rejecting command %i: %s - no such line or no MULTIX configuration set", cmd, mx_get_cmd_name(cmd));
			mx_irqq_enqueue(multix->irqq, route->irq_no_line, line_num);
		} else {
			if (mx_task_is_running(task)) {
				// task not accepted - already running
				LOGID(L_MX, 1, line, "Line is busy, task %i: %s has been rejected", route->task_num, mx_task_name(route->task_num));
				mx_irqq_enqueue(multix->irqq, route->irq_reject, line_num);
			} else {
				mx_task_start(task, arg);
				multix->task_act[route->task_num]++;
				// nothing to do at this point - if task has been accepted
				// TM will handle it and send appropriate IRQ
				LOGID(L_MX, 1, line, "Task %i: \"%s\" has been scheduled for start", route->task_num, mx_task_name(route->task_num));
			}
		}
	}
}

// -----------------------------------------------------------------------
static void mx_ev_handle(struct mx *multix, struct mx_ev *ev)
{
	if (ev->type == MX_EV_INT_RECVD) {
		mx_irqq_advance(multix->irqq);
	} else if (ev->type == MX_EV_CMD) {
		if (ev->cmd == MX_CMD_SETCFG) {
			mx_setcfg(multix, ev->arg);
		} else if (ev->cmd == MX_CMD_TEST) {
			mx_test(multix);
		} else if (ev->cmd == MX_CMD_REQUEUE) {
			mx_irqq_irq_requeue(multix->irqq);
		} else {
			mx_task_router(multix, ev->cmd, ev->line, ev->arg);
		}
	} else if (ev->type == MX_EV_TIMER) {
		LOGID(L_MX, 1, multix, "TIMER TICK");
	} else {
		LOGID(L_MX, 1, multix, "Unknown event type: %i", ev->type);
	}
}

// -----------------------------------------------------------------------
static int mx_get_task_waiting(struct mx *multix)
{
	for (int i=0 ; i<MX_TASK_MAX ; i++) {
		if (multix->task_act[i]) {
			LOGID(L_MX, 3, multix, "Got awaiting task %i: %s", i, mx_task_name(i));
			return i;
		}
	}

	LOGID(L_MX, 3, multix, "No tasks waiting");

	return -1;
}

// -----------------------------------------------------------------------
static int mx_get_line_waiting(struct mx *multix, int task_num)
{
	static int line_num = -1;

	for (int i=0 ; i<multix->log_line_count ; i++) {
		line_num++;
		if (line_num >= multix->log_line_count) {
			line_num = 0;
		}

		struct mx_task *task = multix->log_lines[i]->task + task_num;

		if (mx_task_is_waiting(task)) {
			LOGID(L_MX, 3, multix, "Line %i is waiting for task %i: %s", line_num, task_num, mx_task_name(task_num));
			return line_num;
		}
	}

	LOGID(L_MX, 3, multix, "No line is waiting for task %i: %s", task_num, mx_task_name(task_num));
	return -1;
}

// -----------------------------------------------------------------------
static int mx_task_run(struct mx_line *line, struct mx_task *task, const struct mx_proto_task *proto_task)
{
	uint16_t data[32];
	int irq = MX_IRQ_NONE; // just in case

	// update line task status
	mx_task_activate(task);

	// get highest condition
	unsigned condition = mx_task_get_condition_num(task);

	// get task data if condition is START and protocol requires the data
	// it's protocol job to store that information for other conditions to use
	if ((task->bzaw & MX_COND_START) && (proto_task->input_flen > 0)) {
		if (!mem_mget(0, task->arg, data, proto_task->input_flen)) {
			return MX_IRQ_INPAO;
		}
	}

	// run the protocol handler
	// protocol takes care of all the interaction between the device and line,
	// but it doesn't interact with the CPU directly
	proto_task_f handler = proto_task->fun[condition];
	if (handler) {
		task->bzaw = handler(line, &irq, data);
	} else {
		LOGID(L_MX, 1, line, "EMULATION ERROR: No function to handle given task at condition %i for protocol %s", condition, line->proto->name);
	}

	// if task is done, we may need to update return field
	if ((task->bzaw == MX_COND_NONE) && (proto_task->output_flen > 0)) {
		if (!mem_mput(0, task->arg + proto_task->output_fpos, data, proto_task->output_flen)) {
			return MX_IRQ_INPAO;
		}
	}

	// irq is sent to CPU in mx_process_one_task(), which knows the line number
	return irq;
}

// -----------------------------------------------------------------------
static int mx_process_one_task(struct mx *multix)
{
	int task_num;
	int line_num;

	// find a task awaiting execution
	if ((task_num = mx_get_task_waiting(multix)) == -1) {
		return 0;
	}

	// find first line waiting for the task
	if ((line_num = mx_get_line_waiting(multix, task_num)) == -1) {
		// no line waiting on that task
		multix->task_act[task_num] = 0;
		return 0;
	}

	struct mx_line *line = multix->log_lines[line_num];
	struct mx_task *task = line->task + task_num;
	const struct mx_proto_task *proto_task = line->proto->task + task_num;

	LOGID(L_MX, 3, line, "Running line task %i: %s, condition(-s) met: %s%s%s%s%s%s%s%s",
		task_num,
		mx_task_name(task_num),
		task->bwar & MX_COND_TIMER ? "timer " : "",
		task->bwar & MX_COND_X ? "X " : "",
		task->bwar & MX_COND_WINCH ? "winchester " : "",
		task->bwar & MX_COND_OPRQ ? "oprq " : "",
		task->bwar & MX_COND_ERR ? "error " : "",
		task->bwar & MX_COND_SEND ? "send " : "",
		task->bwar & MX_COND_RECV ? "receive " : "",
		task->bwar & MX_COND_START ? "start" : ""
	);

	// start line task
	int proto_irq = mx_task_run(line, task, proto_task);

	// if task doesn't require any more attention, finish it
	if (task->bwar == MX_COND_NONE) {
		LOGID(L_MX, 3, line, "Line task %s is finished", mx_task_name(task_num));
		multix->task_act[task_num]--;
	}

	// If task wants to send an IRQ, do it here
	if (proto_irq != MX_IRQ_NONE) {
		mx_irqq_enqueue(multix->irqq, proto_irq, line_num);
	}

	return 1;
}

/* -----------------------------------------------------------------------
	MULTIX firmware loop. The main f/w job is to process line tasks.
	Tasks are created/updated in H/W interrupt handler routines.
	Here we simulate hardware interrupts by events in the event queue
	(events are reported by other threads, as CPU od device threads),
	and do interrupt routines by processing events from the queue in the
	main loop, just before starting Task Manager.

	From the Task Manager point of view we don't really care when
	interrupts are served, so we may as well do the processing as follows:

		* Process all events, as we would serve all interrupts during TM work
		* If no event is processed, that's OK
		* Get the most important task
		* If there is no task to process, that means we need to wait for another event
		  (as tasks may only be updated by events)
		* Process the task and loop over, because there may be other events
		  with priority higher than what we were just processing
		  (that's why we don't process all tasks in one pass)
   ----------------------------------------------------------------------- */

// -----------------------------------------------------------------------
static void * mx_main(void *ptr)
{
	struct mx *multix = ptr;
	struct mx_ev *ev;
	int queue_disabled = 0;

	LOGID(L_MX, 3, multix, "Starting main loop thread");

	// initialize MX if 'reset' or, break the loop if 'quit'
	while (!mx_init(multix)) {

		LOGID(L_MX, 3, multix, "Entering main MULTIX loop");

		// MULTIX main loop
		do {
			// process all events
			while ((ev = mx_evq_dequeue(multix->evq))) {
				mx_ev_handle(multix, ev);
				mx_ev_delete(ev);
			}

			// get the highest priority task and process it
			if (!mx_process_one_task(multix)) {
				// no task processed -> wait for a new event,
				queue_disabled = mx_evq_wait(multix->evq);
			}
		// leave the main loop if queue is disabled
		// (meaning emulation quit or multix reset)
		} while (!queue_disabled);

		LOGID(L_MX, 3, multix, "Left main MULTIX loop");
	}

	LOGID(L_MX, 3, multix, "Exiting main loop thread");

	pthread_exit(NULL);
}

// vim: tabstop=4 shiftwidth=4 autoindent
