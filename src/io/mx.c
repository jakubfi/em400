//  Copyright (c) 2013-2014 Jakub Filipowicz <jakubf@gmail.com>
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

#include "io/mx.h"
#include "io/mx_intr.h"
#include "io/mx_cf.h"
#include "io/mx_cmd.h"
#include "io/mx_proto.h"

#include "io/io.h"
#include "io/dev.h"

#include "cfg.h"
#include "errors.h"
#include "log.h"

#define CHAN ((struct mx_chan *)(chan))
#define LLINE (CHAN->lline[llinen])
#define TASK (CHAN->task + taskn)

static struct mx_cmd_table mx_cmd_table[] = {
	{ MX_CMD_ERR_0,		-1,				MX_INTR_IEPS0,	-1				},
	{ MX_CMD_TEST,		-1,				-1,				-1				},
	{ MX_CMD_ATTACH,	MX_TASK_DOL,	MX_INTR_INDOL,	MX_INTR_INKDO	},
	{ MX_CMD_STATUS,	MX_TASK_STR,	MX_INTR_INSTR,	MX_INTR_INKST	},
	{ MX_CMD_TRANSMIT,	MX_TASK_TRA,	MX_INTR_INTRA,	MX_INTR_INKTR	},
	{ MX_CMD_SETCFG,	-1,				-1,				-1				},
	{ MX_CMD_ERR_6,		-1,				MX_INTR_IEPS6,	-1				},
	{ MX_CMD_ERR_7,		-1,				MX_INTR_IEPS7,	-1				},
	{ MX_CMD_ERR_8,		-1,				MX_INTR_IEPS8,	-1				},
	{ MX_CMD_INTRQ,		-1,				-1,				-1				},
	{ MX_CMD_DETACH,	MX_TASK_ODL,	MX_INTR_INODL,	MX_INTR_INKOD	},
	{ MX_CMD_ABORT,		MX_TASK_ABT,	MX_INTR_INABT,	MX_INTR_INKAB	},
	{ MX_CMD_ERR_C,		-1,				MX_INTR_IEPSC,	-1				},
	{ MX_CMD_ERR_D,		-1,				MX_INTR_IEPSD,	-1				},
	{ MX_CMD_ERR_E,		-1,				MX_INTR_IEPSE,	-1				},
	{ MX_CMD_ERR_F,		-1,				MX_INTR_IEPSF,	-1				}
};

static const char *mx_task_names[] = {
	"report status",
	"detach line",
	"OPRQ",
	"transmit",
	"abort transmission",
	"attach line"
};

char *mx_chan_cmd_names[] = {
	"RESET",
	"INTSPEC",
	"EXISTS",
	"INVALID"
};

char *mx_cmd_names[] = {
	"ERR_0",
	"TEST",
	"ATTACH",
	"STATUS",
	"TRANSMIT",
	"SETCFG",
	"ERR_6",
	"ERR_7",
	"ERR_8",
	"INTRQ",
	"DETACH",
	"CANCEL",
	"ERR_C",
	"ERR_D",
	"ERR_E",
	"ERR_F",
	"EM400_QUIT",
	"EM400_NONE",
	"EM400_FRESET"
};

static void * mx_cmd_receiver(void *ptr);
static void * mx_task_manager(void *ptr);

// -----------------------------------------------------------------------
struct chan * mx_create(struct cfg_unit *units)
{
	int res;

	struct mx_chan *chan = calloc(1, sizeof(struct mx_chan));
	if (!chan) {
		gerr = E_ALLOC;
		goto cleanup;
	}

	// init interrupt queue
	pthread_mutex_init(&chan->intr_mutex, NULL);

	// init task manager
	pthread_mutex_init(&chan->task_mutex, NULL);
	pthread_cond_init(&chan->task_cond, NULL);
	res = pthread_create(&chan->task_manager_th, NULL, mx_task_manager, chan);
	if (res != 0) {
		gerr = E_THREAD;
		goto cleanup;
	}

	// init command receiver
	chan->cmd_recv = MX_CMD_NONE;
	pthread_mutex_init(&chan->cmd_recv_mutex, NULL);
	pthread_cond_init(&chan->cmd_recv_cond, NULL);
	res = pthread_create(&chan->cmd_recv_th, NULL, mx_cmd_receiver, chan);
	if (res != 0) {
		gerr = E_THREAD;
		goto cleanup;
	}

	// create devices connected to physical lines
	struct cfg_unit *cunit = units;
	while (cunit) {
		LOG(L_MX, 1, "    Device %i: %s", cunit->num, cunit->name);
		CHAN->pline[cunit->num].device = dev_create(cunit->name, cunit->args, NULL);
		// TODO: error handling
		cunit = cunit->next;
	}

	return (struct chan*) chan;

cleanup:
	mx_shutdown((struct chan*) chan);
	return NULL;
}

// -----------------------------------------------------------------------
void mx_shutdown(struct chan *chan)
{
	// stop command receiver
	pthread_mutex_lock(&CHAN->cmd_recv_mutex);
	CHAN->cmd_recv = MX_CMD_QUIT;
	pthread_cond_signal(&CHAN->cmd_recv_cond);
	pthread_mutex_unlock(&CHAN->cmd_recv_mutex);
	pthread_join(CHAN->cmd_recv_th, NULL);
	pthread_mutex_destroy(&CHAN->cmd_recv_mutex);

	// stop task manager
	pthread_mutex_lock(&CHAN->task_mutex);
	CHAN->task_quit = 1;
	mx_task_clear_all(CHAN);
	pthread_cond_signal(&CHAN->task_cond);
	pthread_mutex_unlock(&CHAN->task_mutex);
	pthread_join(CHAN->task_manager_th, NULL);
	pthread_mutex_destroy(&CHAN->task_mutex);

	// stop and deconfigure lines, protos and devices
	pthread_mutex_lock(&CHAN->conf_mutex);
	for (int i=0 ; i<MX_LINE_MAX ; i++) {
		if (CHAN->pline[i].proto) {
			mx_proto_destroy(CHAN->pline[i].proto);
		}
		dev_close(CHAN->pline[i].device);
		CHAN->lline[i] = NULL;
	}
	pthread_mutex_unlock(&CHAN->conf_mutex);
	pthread_mutex_destroy(&CHAN->conf_mutex);

	// drop interrupt queue
	pthread_mutex_lock(&CHAN->intr_mutex);
	mx_intr_clearq(CHAN);
	CHAN->intr_head = CHAN->intr_tail = NULL;
	pthread_mutex_unlock(&CHAN->intr_mutex);
	pthread_mutex_destroy(&CHAN->intr_mutex);

	free(chan);
}

// -----------------------------------------------------------------------
void mx_reset(struct chan *chan)
{
	// initiate asynchronous reset
	// (actual work done in command receiver thread)
	mx_cmd(chan, IO_IN, 0, NULL);
}

// -----------------------------------------------------------------------
// ---- TASK MANAGER
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// call with task_mutex locked
void mx_task_clear(struct mx_chan *chan, int taskn)
{
	chan->task[taskn].reported = 0;
	for (int l=0 ; l<MX_LINE_MAX ; l++) {
		chan->task[taskn].line[l].sleep = MX_COND_NONE;
		chan->task[taskn].line[l].condition = MX_COND_NONE;
	}
}

// -----------------------------------------------------------------------
// call with task_mutex locked
void mx_task_clear_all(struct mx_chan *chan)
{
	for (int t=0 ; t<MX_TASK_MAX ; t++) {
		mx_task_clear(chan, t);
	}
}

// -----------------------------------------------------------------------
// thread: task manager
// with task_mutex locked
void mx_task_attach(struct chan *chan, int llinen)
{
	LOG(L_MX, 3, "MULTIX (ch:%i lline:%i) ATTACH", chan->num, llinen);
	int res;
	int attached;
	uint16_t data[8];

	pthread_mutex_lock(&CHAN->conf_mutex);
	attached = LLINE->attached;
	pthread_mutex_unlock(&CHAN->conf_mutex);

	if (attached) {
		mx_int(CHAN, llinen, MX_INTR_INDOL);
		return;
	}

	mem_mget(0, LLINE->addr, data, 8);

	res = LLINE->proto->attach(LLINE, data);
	if (res) {
		mx_int(CHAN, llinen, MX_INTR_INDOL);
	} else {
		pthread_mutex_lock(&CHAN->conf_mutex);
		LLINE->attached = 1;
		pthread_mutex_unlock(&CHAN->conf_mutex);
		mx_int(CHAN, llinen, MX_INTR_IDOLI);
	}
}

// -----------------------------------------------------------------------
// thread: task manager
// with task_mutex locked
void mx_task_detach(struct chan *chan, int llinen)
{
	LOG(L_MX, 3, "MULTIX (ch:%i lline:%i) DETACH", chan->num, llinen);
	// if transmission is active in the line
	if (CHAN->task[MX_TASK_TRA].line[llinen].sleep || (CHAN->task[MX_TASK_TRA].line[llinen].condition & 1)) {
		mx_int(CHAN, llinen, MX_INTR_INODL);
	} else {
		LLINE->proto->detach(LLINE);
		pthread_mutex_lock(&CHAN->conf_mutex);
		LLINE->attached = 0;
		pthread_mutex_unlock(&CHAN->conf_mutex);
		mx_int(CHAN, llinen, MX_INTR_IODLI);
	}
}

// -----------------------------------------------------------------------
// thread: task manager
// with task_mutex locked
static int mx_task_get(struct mx_chan *chan)
{
	if (chan->task_quit) {
		return MX_TASK_QUIT;
	}

	for (int t=0 ; t<MX_TASK_MAX ; t++) {
		if (chan->task[t].reported) {
			return t;
		}
	}

	return MX_TASK_NONE;
}

// -----------------------------------------------------------------------
// thread: task manager
// called by task manager with task_mutex locked
void mx_task_process(struct chan *chan, int taskn, int llinen, int conditions)
{
	LOG(L_MX, 3, "MULTIX (ch:%i lline:%i) task manager processing task %i: %s met conditions: [%c%c%c%c%c%c%c%c]",
		chan->num,
		llinen, taskn,
		mx_task_names[taskn],
		conditions & MX_COND_WATIM ? 'C' : '.',
		conditions & MX_COND_X     ? '!' : '.',
		conditions & MX_COND_WAFWI ? 'W' : '.',
		conditions & MX_COND_WAOPR ? 'O' : '.',
		conditions & MX_COND_WAAWA ? 'E' : '.',
		conditions & MX_COND_WANAD ? 'T' : '.',
		conditions & MX_COND_WAODB ? 'R' : '.',
		conditions & MX_COND_START ? 'S' : '.'
	);

	switch (taskn) {
		case MX_TASK_STR:
			break;
		case MX_TASK_ODL:
			mx_task_detach(chan, llinen);
			break;
		case MX_TASK_ORQ:
			mx_int(CHAN, llinen, MX_INTR_IOPRU);
			// TODO: co jeszcze tutaj zrobic?
			break;
		case MX_TASK_TRA:
			break;
		case MX_TASK_ABT:
			break;
		case MX_TASK_DOL:
			mx_task_attach(chan, llinen);
			break;
		default:
			break;
	}

	// TODO, this is temporary
	TASK->line[llinen].condition = 0;
}

// -----------------------------------------------------------------------
// thread: task manager
// mx main loop
static void * mx_task_manager(void *ptr)
{
	struct chan *chan = ptr;

	static int llinen = -1; // previously processed line
	int lc;
	int condition;

	while (1) {
		pthread_mutex_lock(&CHAN->task_mutex);

		int taskn = mx_task_get(CHAN);

		// wait until receiver sends a command
		while (taskn == MX_TASK_NONE) {
			LOG(L_MX, 3, "MULTIX (ch:%i) task manager waiting for task...", chan->num);
			pthread_cond_wait(&CHAN->task_cond, &CHAN->task_mutex);
			taskn = mx_task_get(CHAN);
		}

		if (taskn == MX_TASK_QUIT) {
			// stop processor thread
			pthread_mutex_unlock(&CHAN->task_mutex);
			break;
		}

		// choose line
		LOG(L_MX, 3, "MULTIX (ch:%i) task manager woke up", chan->num);
		for (lc=0 ; lc<MX_LINE_MAX ; lc++) {
			llinen++;
			if (llinen >= MX_LINE_MAX) llinen = 0;
			condition = (TASK->line[llinen].sleep | 1) & TASK->line[llinen].condition;
			if (condition != 0) {
				// process task...
				mx_task_process(chan, taskn, llinen, condition);
				// ...and give task manager another try (there may be more important task to do)
				break;
			}
		}

		// there were no lines requiring attention for this task
		if (lc >= MX_LINE_MAX) {
			LOG(L_MX, 3, "MULTIX (ch:%i) task manager found no active tasks", chan->num);
			mx_task_clear(CHAN, taskn);
		}

		pthread_mutex_unlock(&CHAN->task_mutex);
	}

	LOG(L_MX, 3, "MULTIX (ch:%i) closing task manager", chan->num);
	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
// ---- COMMAND PROCESSING
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// thread: command receiver
// try accepting command and either:
//  * do the work (reset, illegal commands, setconf, interrupt requeue)
//  * or prepare a task (for task manager thread)
// call with cmd_recv_mutex locked
static void mx_cmd_process(struct chan *chan, int cmd, int llinen, uint16_t addr)
{
	LOG(L_MX, 1, "MULTIX (ch:%i, lline:%i) receiver accepting line command %i: %s", chan->num, llinen, cmd, mx_cmd_names[cmd]);

	pthread_mutex_lock(&CHAN->task_mutex);

	struct mx_cmd_table *cmd_task = mx_cmd_table + cmd;

	switch (cmd) {
		case MX_CMD_QUIT:
			CHAN->task_quit = 1;
			pthread_cond_signal(&CHAN->task_cond);
			break;
		case MX_CMD_FRESET:
			mx_cmd_reset(chan);
			break;
		case MX_CMD_TEST:
			mx_cmd_test(chan);
			break;
		case MX_CMD_SETCFG:
			mx_cmd_confset(chan, addr);
			break;
		case MX_CMD_INTRQ:
			mx_cmd_int_requeue(chan);
			break;
		case MX_CMD_ERR_0:
		case MX_CMD_ERR_6:
		case MX_CMD_ERR_7:
		case MX_CMD_ERR_8:
		case MX_CMD_ERR_C:
		case MX_CMD_ERR_D:
		case MX_CMD_ERR_E:
		case MX_CMD_ERR_F:
			mx_cmd_illegal(chan, llinen, cmd_task->intr_reject);
			break;
		case MX_CMD_ATTACH:
		case MX_CMD_STATUS:
		case MX_CMD_TRANSMIT:
		case MX_CMD_DETACH:
		case MX_CMD_ABORT:
			pthread_mutex_lock(&CHAN->conf_mutex);

			struct mx_task_line *task = CHAN->task + cmd_task->task_n;

			// if conf is not set or line is not configured
			if (!CHAN->conf_set || !LLINE) {
				mx_int(CHAN, llinen, cmd_task->intr_noline);
			// if task is already reported for a line or running on a line or waiting for something
			} else if ((task->line[llinen].condition & MX_COND_START) || task->line[llinen].sleep) {
				mx_int(CHAN, llinen, cmd_task->intr_reject);
			// we are free to report a line task
			} else {
				task->reported = 1;
				task->line[llinen].condition |= MX_COND_START;
				LLINE->addr = addr;
	            pthread_cond_signal(&CHAN->task_cond);
			}

			pthread_mutex_unlock(&CHAN->conf_mutex);
			break;
		default:
			LOG(L_MX, 1, "MULTIX (ch:%i, lline:%i) UNHANDLED COMMAND: %s (this shouldn't happen)", chan->num, llinen, cmd);
			break;
	}

	pthread_mutex_unlock(&CHAN->task_mutex);
}

// -----------------------------------------------------------------------
// thread: command receiver
// mx IPOST interrupt handler
static void * mx_cmd_receiver(void *ptr)
{
	struct chan *chan = ptr;

	while (!CHAN->task_quit) {
		pthread_mutex_lock(&CHAN->cmd_recv_mutex);

		// wait until CPU sends a command
		while (CHAN->cmd_recv == MX_CMD_NONE) {
			LOG(L_MX, 3, "MULTIX (ch:%i) receiver awaiting command...", chan->num);
			pthread_cond_wait(&CHAN->cmd_recv_cond, &CHAN->cmd_recv_mutex);
		}

		mx_cmd_process(chan, CHAN->cmd_recv, CHAN->cmd_recv_llinen, CHAN->cmd_recv_addr);

		CHAN->cmd_recv = MX_CMD_NONE;
		pthread_mutex_unlock(&CHAN->cmd_recv_mutex);
	}

	LOG(L_MX, 3, "MULTIX (ch:%i) closing command receiver", chan->num);
	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
// thread: CPU
// try forwarding command to command receiver thread
static int mx_cmd_forward(struct chan *chan, int cmd, int llinen, uint16_t addr)
{
	// check if we can receive a command (cmd_receiver() is free and previous command has been taken care of)
	if ((pthread_mutex_trylock(&CHAN->cmd_recv_mutex) == 0) && (CHAN->cmd_recv != MX_CMD_NONE)) {
		LOG(L_MX, 1, "MULTIX (ch:%i, lline:%i) forwarding command %i: %s", chan->num, llinen, cmd, mx_cmd_names[cmd]);
		CHAN->cmd_recv = cmd;
		CHAN->cmd_recv_llinen = llinen;
		CHAN->cmd_recv_addr = addr;
		pthread_cond_signal(&CHAN->cmd_recv_cond);
		pthread_mutex_unlock(&CHAN->cmd_recv_mutex);
		return IO_OK;
	} else {
		// reply with "engaged" if not ready for a command
		LOG(L_MX, 1, "MULTIX (ch:%i, lline:%i) rejecting command %i: %s", chan->num, llinen, cmd, mx_cmd_names[cmd]);
		return IO_EN;
	}
}

// -----------------------------------------------------------------------
// thread: CPU
// mx command entry
int mx_cmd(struct chan *chan, int dir, uint16_t n_arg, uint16_t *r_arg)
{
	unsigned cmd		= ((n_arg & 0b1110000000000000) >> 13) | ((dir&1) << 3);
	unsigned chan_cmd	=  (n_arg & 0b0001100000000000) >> 11;
	unsigned llinen		=  (n_arg & 0b0001111111100000) >> 5;

	// channel commands
	if (cmd == MX_CMD_CHAN) {
		LOG(L_MX, 1, "MULTIX (ch:%i) incomming channel command %i: %s", chan->num, chan_cmd, mx_chan_cmd_names[chan_cmd]);
		switch (chan_cmd) {
			// 'int spec' needs to be ready upon I/O end
			case MX_CMD_INTSPEC:
				mx_cmd_intspec(chan, r_arg);
				return IO_OK;
			// 'exists' does nothing, just returns OK
			case MX_CMD_EXISTS:
				return IO_OK;
			// 'reset' returns OK and resets mx asynchronously
			case MX_CMD_RESET:
				pthread_mutex_lock(&CHAN->cmd_recv_mutex);
				CHAN->conf_set = 0;
				CHAN->cmd_recv_llinen = -1;
				CHAN->cmd_recv = MX_CMD_FRESET;
				pthread_cond_signal(&CHAN->cmd_recv_cond);
				pthread_mutex_unlock(&CHAN->cmd_recv_mutex);
				return IO_OK;
			// handle other commands (only MX_CMD_INVALID in fact) as illegal in receiver thread
			default:
				return mx_cmd_forward(chan, MX_CMD_ERR_8, -1, -1);
		}
	// general and line commands
	} else {
		return mx_cmd_forward(chan, cmd, llinen, *r_arg);
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
