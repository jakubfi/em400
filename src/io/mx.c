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

#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>

#include "mem/mem.h"

#include "io/mx.h"
#include "io/mx_intr.h"
#include "io/mx_cf.h"
#include "io/mx_cmd.h"
#include "io/mx_proto.h"

#include "io/dev.h"

#include "cfg.h"
#include "errors.h"
#include "log.h"

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
static void * mx_main(void *ptr);

// -----------------------------------------------------------------------
void * mx_create(int num, struct cfg_unit *units)
{
	int res;

	struct mx *multix = calloc(1, sizeof(struct mx));
	if (!multix) {
		gerr = E_ALLOC;
		goto cleanup;
	}

	multix->num = num;

	// create devices connected to physical lines
	struct cfg_unit *cunit = units;
	while (cunit) {
		LOG(L_MX, 1, "    Device %i: %s", cunit->num, cunit->name);
		multix->pline[cunit->num].device = dev_create(cunit->name, cunit->args, NULL);
		// TODO: error handling
		cunit = cunit->next;
	}

	// init interrupt queue
	pthread_mutex_init(&multix->intr_mutex, NULL);

	// init command receiver
	multix->cmd_recv = MX_CMD_RESET;
	pthread_mutex_init(&multix->cmd_recv_mutex, NULL);
	pthread_cond_init(&multix->cmd_recv_cond, NULL);
	res = pthread_create(&multix->cmd_recv_th, NULL, mx_cmd_receiver, multix);
	if (res != 0) {
		gerr = E_THREAD;
		goto cleanup;
	}

	// init task manager
	multix->task_em = MX_TASK_RESET;
	pthread_mutex_init(&multix->task_mutex, NULL);
	pthread_cond_init(&multix->task_cond, NULL);
	res = pthread_create(&multix->task_th, NULL, mx_main, multix);
	if (res != 0) {
		gerr = E_THREAD;
		goto cleanup;
	}

	return (void *) multix;

cleanup:
	mx_shutdown((void *) multix);
	return NULL;
}

// -----------------------------------------------------------------------
void mx_shutdown(void *ch)
{
	struct mx *multix = (struct mx *) ch;

	// stop command receiver
	pthread_mutex_lock(&multix->cmd_recv_mutex);
	multix->cmd_recv = MX_CMD_QUIT;

	pthread_cond_signal(&multix->cmd_recv_cond);
	pthread_mutex_unlock(&multix->cmd_recv_mutex);
	pthread_join(multix->cmd_recv_th, NULL);

	// stop task manager
	pthread_mutex_lock(&multix->task_mutex);
	multix->task_em = MX_TASK_QUIT;
	mx_task_clear_all(multix);
	pthread_cond_signal(&multix->task_cond);
	pthread_mutex_unlock(&multix->task_mutex);
	pthread_join(multix->task_th, NULL);

	// stop and deconfigure lines, protos and devices
	pthread_mutex_lock(&multix->conf_mutex);
	for (int i=0 ; i<MX_LINE_MAX ; i++) {
		if (multix->pline[i].proto) {
			mx_proto_destroy(multix->pline[i].proto);
		}
		dev_close(multix->pline[i].device);
		multix->lline[i] = NULL;
	}
	pthread_mutex_unlock(&multix->conf_mutex);

	// clear interrupt queue
	pthread_mutex_lock(&multix->intr_mutex);
	mx_intr_clearq(multix);
	multix->intr_head = multix->intr_tail = NULL;
	pthread_mutex_unlock(&multix->intr_mutex);

	pthread_mutex_destroy(&multix->cmd_recv_mutex);
	pthread_cond_destroy(&multix->cmd_recv_cond);
	pthread_mutex_destroy(&multix->task_mutex);
	pthread_cond_destroy(&multix->task_cond);
	pthread_mutex_destroy(&multix->conf_mutex);
	pthread_mutex_destroy(&multix->intr_mutex);

	free(multix);
}

// -----------------------------------------------------------------------
void mx_reset(void *ch)
{
	struct mx *multix = (struct mx *) ch;

	// Here, we only initialize MULTIX reset, actual reset happend in main loop
	LOG(L_MX, 1, "Initiating MULTIX reset.");

	// first, make sure no command is received after we return from reset
	pthread_mutex_lock(&multix->cmd_recv_mutex);
	multix->cmd_recv = MX_CMD_RESET;
	pthread_cond_signal(&multix->cmd_recv_cond);
	pthread_mutex_unlock(&multix->cmd_recv_mutex);

	// then, initiate actual reset in main loop (task manager thread)
	pthread_mutex_lock(&multix->task_mutex);
	multix->task_em = MX_TASK_RESET;
	pthread_cond_signal(&multix->task_cond);
	pthread_mutex_unlock(&multix->task_mutex);

	LOG(L_MX, 1, "MULTIX reset initiated");
}

// -----------------------------------------------------------------------
// ---- TASK MANAGER
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// call with task_mutex locked
void mx_task_clear(struct mx *multix, int taskn)
{
	multix->task[taskn].reported = 0;
	for (int l=0 ; l<MX_LINE_MAX ; l++) {
		multix->task[taskn].line[l].sleep = MX_COND_NONE;
		multix->task[taskn].line[l].condition = MX_COND_NONE;
	}
}

// -----------------------------------------------------------------------
// call with task_mutex locked
void mx_task_clear_all(struct mx *multix)
{
	for (int t=0 ; t<MX_TASK_MAX ; t++) {
		mx_task_clear(multix, t);
	}
}

// -----------------------------------------------------------------------
// thread: task manager
// with task_mutex locked
void mx_task_attach(struct mx *multix, int llinen)
{
	LOG(L_MX, 3, "MULTIX (ch:%i lline:%i) ATTACH", multix->num, llinen);
	int res;
	int attached;
	uint16_t data[8];

	struct mx_line *lline = multix->lline[llinen];

	pthread_mutex_lock(&multix->conf_mutex);
	attached = lline->attached;
	pthread_mutex_unlock(&multix->conf_mutex);

	if (attached) {
		mx_int(multix, llinen, MX_INTR_INDOL);
		return;
	}

	mem_mget(0, lline->addr, data, 8);

	res = lline->proto->attach(lline, data);
	if (res) {
		mx_int(multix, llinen, MX_INTR_INDOL);
	} else {
		pthread_mutex_lock(&multix->conf_mutex);
		lline->attached = 1;
		pthread_mutex_unlock(&multix->conf_mutex);
		mx_int(multix, llinen, MX_INTR_IDOLI);
	}
}

// -----------------------------------------------------------------------
// thread: task manager
// with task_mutex locked
void mx_task_detach(struct mx *multix, int llinen)
{
	LOG(L_MX, 3, "MULTIX (ch:%i lline:%i) DETACH", multix->num, llinen);

	struct mx_line *lline = multix->lline[llinen];

	// if transmission is active in the line
	if (multix->task[MX_TASK_TRA].line[llinen].sleep || (multix->task[MX_TASK_TRA].line[llinen].condition & 1)) {
		mx_int(multix, llinen, MX_INTR_INODL);
	} else {
		lline->proto->detach(lline);
		pthread_mutex_lock(&multix->conf_mutex);
		lline->attached = 0;
		pthread_mutex_unlock(&multix->conf_mutex);
		mx_int(multix, llinen, MX_INTR_IODLI);
	}
}

// -----------------------------------------------------------------------
// thread: task manager
// with task_mutex locked
static int mx_task_get(struct mx *multix)
{
	if (multix->task_em != MX_TASK_NONE) {
		return multix->task_em;
	}

	for (int t=0 ; t<MX_TASK_MAX ; t++) {
		if (multix->task[t].reported) {
			return t;
		}
	}

	return MX_TASK_NONE;
}

// -----------------------------------------------------------------------
// thread: task manager
// called by task manager with task_mutex locked
void mx_task_process(struct mx *multix, int taskn, int llinen, int conditions)
{
	LOG(L_MX, 3, "MULTIX (ch:%i lline:%i) task manager processing task %i: %s met conditions: [%c%c%c%c%c%c%c%c]",
		multix->num,
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
			mx_task_detach(multix, llinen);
			break;
		case MX_TASK_ORQ:
			mx_int(multix, llinen, MX_INTR_IOPRU);
			// TODO: co jeszcze tutaj zrobic?
			break;
		case MX_TASK_TRA:
			break;
		case MX_TASK_ABT:
			break;
		case MX_TASK_DOL:
			mx_task_attach(multix, llinen);
			break;
		default:
			break;
	}

	// TODO, this is temporary
	multix->task[taskn].line[llinen].condition = 0;
}

// -----------------------------------------------------------------------
// thread: main loop
// mx task manager
static int mx_task_manager(struct mx *multix)
{
	static int llinen = -1; // previously processed line
	int lc;
	int condition;
	int taskn;

	pthread_mutex_lock(&multix->task_mutex);

	while (1) {

		// wait until command receiver prepares a task
		while ((taskn = mx_task_get(multix)) == MX_TASK_NONE) {
			LOG(L_MX, 3, "MULTIX (ch:%i) task manager waiting for task...", multix->num);
			pthread_cond_wait(&multix->task_cond, &multix->task_mutex);
			LOG(L_MX, 3, "MULTIX (ch:%i) task manager woken up", multix->num);
		}

		if (taskn == MX_TASK_QUIT) {
			LOG(L_MX, 3, "MULTIX (ch:%i) task manager quits", multix->num);
			break;
		} else if (taskn == MX_TASK_RESET) {
			LOG(L_MX, 3, "MULTIX (ch:%i) task manager resets", multix->num);
			break;
		}

		// choose line
		for (lc=0 ; lc<MX_LINE_MAX ; lc++) {
			llinen++;
			struct mx_task_line *task = multix->task + taskn;
			if (llinen >= MX_LINE_MAX) llinen = 0;
			condition = (task->line[llinen].sleep | 1) & task->line[llinen].condition;
			if (condition != 0) {
				// process task...
				mx_task_process(multix, taskn, llinen, condition);
				// ...and give task manager another try (there may be more important task to do)
				break;
			}
		}

		// there were no lines requiring attention for this task
		if (lc >= MX_LINE_MAX) {
			LOG(L_MX, 3, "MULTIX (ch:%i) task manager found no active tasks", multix->num);
			mx_task_clear(multix, taskn);
		}
	}

	pthread_mutex_unlock(&multix->task_mutex);

	return taskn;
}

// -----------------------------------------------------------------------
// thread: main MULTIX loop
// initialization routine (called on startup and reset)
static void mx_initialize(struct mx * multix)
{
	LOG(L_MX, 2, "MULTIX (ch:%i) initializing...", multix->num);

	pthread_mutex_lock(&multix->conf_mutex);

	// deconfigure
	multix->conf_set = 0;

	// clear lines configuration
	for (int i=0 ; i<MX_LINE_MAX ; i++) {
		mx_proto_destroy(multix->pline[i].proto);
		multix->pline[i].proto = NULL;
		multix->pline[i].used = 0;
		multix->pline[i].dir = 0;
		multix->pline[i].type = 0;
		multix->pline[i].attached = 0;
		multix->lline[i] = NULL;
	}

	pthread_mutex_unlock(&multix->conf_mutex);

	// TODO: disable interrupts? - chyba nie, tylko task manager wysyła przerwania (?)
	// TODO: (wysyła również receiver)

	// let's say it takes 150 ms to reset MULTIX
	const long reset_wait_msec = 150;

	pthread_mutex_lock(&multix->task_mutex);

	// if multix->task_em is anything other than MX_TASK_RESET,
	// that in practice may only mean MX_TASK_QUIT
	// Anyway, skip reset and let task manager handle it

	while (multix->task_em == MX_TASK_RESET) {
		LOG(L_MX, 2, "MULTIX (ch:%i) initialization delay: %i ms", multix->num, reset_wait_msec);
		multix->task_em = MX_TASK_NONE;
		struct timespec abstime;
		clock_gettime(CLOCK_REALTIME, &abstime);
		long new_nsec = abstime.tv_nsec + reset_wait_msec * 1000000L;
		abstime.tv_sec += new_nsec / 1000000000L;
		abstime.tv_nsec = new_nsec % 1000000000L;

		pthread_cond_timedwait(&multix->task_cond, &multix->task_mutex, &abstime);
		LOG(L_MX, 2, "MULTIX (ch:%i) reset delay woken up", multix->num);

		// if above wakes up again, and multix->task_em is MX_TASK_RESET again,
		// it means that another reset came in and we need to reschedule fake reset routine
	}

	// drop all tasks
	mx_task_clear_all(multix);

	pthread_mutex_unlock(&multix->task_mutex);

	// enable command receiving again (use lock, not trylock, we must clear it)
	pthread_mutex_lock(&multix->cmd_recv_mutex);
	multix->cmd_recv = MX_CMD_NONE;
	pthread_cond_signal(&multix->cmd_recv_cond);
	pthread_mutex_unlock(&multix->cmd_recv_mutex);

	LOG(L_MX, 2, "MULTIX (ch:%i) initialization done", multix->num);

	// report IWYZE (this will also clear intr queue
	mx_int(multix, 0, MX_INTR_IWYZE);
}

// -----------------------------------------------------------------------
// thread: main MULTIX loop
static void * mx_main(void *ptr)
{
	struct mx *multix = ptr;

	LOG(L_MX, 3, "MULTIX (ch:%i) Starting main loop thread", multix->num);

	while (1) {
		// initialize MULTIX
		mx_initialize(multix);

		// run task manager, quit, if ordered to
		if (mx_task_manager(ptr) == MX_TASK_QUIT) {
			LOG(L_MX, 3, "MULTIX (ch:%i) main loop thread quits", multix->num);
			break;
		}
	}

	LOG(L_MX, 3, "MULTIX (ch:%i) exiting main loop thread", multix->num);
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
static void mx_cmd_process(struct mx *multix, int cmd, int llinen, uint16_t addr)
{
	LOG(L_MX, 1, "MULTIX (ch:%i, lline:%i) receiver accepting line command %i: %s", multix->num, llinen, cmd, mx_cmd_names[cmd]);

	// TODO: task mutex for general commands? why?
	pthread_mutex_lock(&multix->task_mutex);

	struct mx_cmd_table *cmd_task = mx_cmd_table + cmd;

	switch (cmd) {
		case MX_CMD_TEST:
			mx_cmd_test(multix);
			break;
		case MX_CMD_SETCFG:
			mx_cmd_confset(multix, addr);
			break;
		case MX_CMD_INTRQ:
			mx_cmd_int_requeue(multix);
			break;
		case MX_CMD_ERR_0:
		case MX_CMD_ERR_6:
		case MX_CMD_ERR_7:
		case MX_CMD_ERR_8:
		case MX_CMD_ERR_C:
		case MX_CMD_ERR_D:
		case MX_CMD_ERR_E:
		case MX_CMD_ERR_F:
			mx_cmd_illegal(multix, llinen, cmd_task->intr_reject);
			break;
		case MX_CMD_ATTACH:
		case MX_CMD_STATUS:
		case MX_CMD_TRANSMIT:
		case MX_CMD_DETACH:
		case MX_CMD_ABORT:
			pthread_mutex_lock(&multix->conf_mutex);

			struct mx_task_line *task = multix->task + cmd_task->task_n;
			struct mx_line *lline = multix->lline[llinen];

			// if conf is not set or line is not configured
			if (!multix->conf_set || !lline) {
				mx_int(multix, llinen, cmd_task->intr_noline);
			// if task is already reported for a line or running on a line or waiting for something
			} else if ((task->line[llinen].condition & MX_COND_START) || task->line[llinen].sleep) {
				mx_int(multix, llinen, cmd_task->intr_reject);
			// we are free to report a line task
			} else {
				task->reported = 1;
				task->line[llinen].condition |= MX_COND_START;
				lline->addr = addr;
	            pthread_cond_signal(&multix->task_cond);
			}

			pthread_mutex_unlock(&multix->conf_mutex);
			break;
		default:
			LOG(L_MX, 1, "MULTIX (ch:%i, lline:%i) UNHANDLED COMMAND: %s (this shouldn't happen)", multix->num, llinen, cmd);
			break;
	}

	pthread_mutex_unlock(&multix->task_mutex);
}

// -----------------------------------------------------------------------
// thread: command receiver
// mx IPOST interrupt handler
static void * mx_cmd_receiver(void *ptr)
{
	struct mx *multix = ptr;

	pthread_mutex_lock(&multix->cmd_recv_mutex);

	while (1) {
		// wait until CPU sends a command
		while (multix->cmd_recv == MX_CMD_NONE) {
			LOG(L_MX, 3, "MULTIX (ch:%i) receiver awaiting command...", multix->num);
			pthread_cond_wait(&multix->cmd_recv_cond, &multix->cmd_recv_mutex);
		}

		if (multix->cmd_recv == MX_CMD_QUIT) {
			LOG(L_MX, 3, "MULTIX (ch:%i) command receiver quits", multix->num);
			break;
		} else if (multix->cmd_recv == MX_CMD_RESET) {
			// Keep multix->cmd_recv unchanged, so subsequent commands from CPU fail with EN.
			// Command receiver is now halted until reset is handled in the main loop
			// and multix->cmd_recv is changed to MX_CMD_NONE there
			while (multix->cmd_recv == MX_CMD_RESET) {
				LOG(L_MX, 3, "MULTIX (ch:%i) receiver waiting for reset to complete...", multix->num);
				pthread_cond_wait(&multix->cmd_recv_cond, &multix->cmd_recv_mutex);
			}
		} else {
			mx_cmd_process(multix, multix->cmd_recv, multix->cmd_recv_llinen, multix->cmd_recv_addr);
			multix->cmd_recv = MX_CMD_NONE;
		}
	}

	pthread_mutex_unlock(&multix->cmd_recv_mutex);

	LOG(L_MX, 3, "MULTIX (ch:%i) exiting command receiver thread", multix->num);
	pthread_exit(NULL);
}

// -----------------------------------------------------------------------
// thread: CPU
// try forwarding command to command receiver thread
static int mx_cmd_forward(struct mx *chan, int cmd, int llinen, uint16_t addr)
{
	// check if we can receive a command
	// is cmd_receiver() free?
	if (pthread_mutex_trylock(&chan->cmd_recv_mutex) == 0) {
		// has previous command been taken care of?
		if (chan->cmd_recv == MX_CMD_NONE) {
			LOG(L_MX, 1, "MULTIX (ch:%i, lline:%i) forwarding command %i: %s", chan->num, llinen, cmd, mx_cmd_names[cmd]);
			chan->cmd_recv = cmd;
			chan->cmd_recv_llinen = llinen;
			chan->cmd_recv_addr = addr;
			pthread_cond_signal(&chan->cmd_recv_cond);
			pthread_mutex_unlock(&chan->cmd_recv_mutex);
			return IO_OK;
		} else {
			pthread_mutex_unlock(&chan->cmd_recv_mutex);
		}
	}

	LOG(L_MX, 1, "MULTIX (ch:%i, lline:%i) receiver busy, rejecting command %i: %s", chan->num, llinen, cmd, mx_cmd_names[cmd]);

	// reply with "engaged" - not yet ready for next command
	return IO_EN;
}

// -----------------------------------------------------------------------
// thread: CPU
// mx command entry
int mx_cmd(void *ch, int dir, uint16_t n_arg, uint16_t *r_arg)
{
	struct mx *multix = (struct mx *) ch;

	unsigned cmd		= ((n_arg & 0b1110000000000000) >> 13) | ((dir&1) << 3);
	unsigned chan_cmd	=  (n_arg & 0b0001100000000000) >> 11;
	unsigned llinen		=  (n_arg & 0b0001111111100000) >> 5;

	// channel commands
	if (cmd == MX_CMD_CHAN) {
		LOG(L_MX, 1, "MULTIX (ch:%i) incomming multixnel command %i: %s", multix->num, chan_cmd, mx_chan_cmd_names[chan_cmd]);
		switch (chan_cmd) {
			// 'int spec' needs to be ready upon I/O end
			case MX_CMD_INTSPEC:
				mx_cmd_intspec(multix, r_arg);
				return IO_OK;
			// 'exists' does nothing, just returns OK
			case MX_CMD_EXISTS:
				return IO_OK;
			// 'reset' initiates reset and returns OK (actual reset is done in main thread)
			case MX_CMD_RESET:
				mx_reset(multix);
				return IO_OK;
			// handle other commands (only MX_CMD_INVALID in fact) as illegal in receiver thread
			default:
				return mx_cmd_forward(multix, MX_CMD_ERR_8, -1, -1);
		}
	// general and line commands
	} else {
		return mx_cmd_forward(multix, cmd, llinen, *r_arg);
	}
}

struct chan_drv mx_chan_driver = {
	.name = "multix",
	.create = mx_create,
	.shutdown = mx_shutdown,
	.reset = mx_reset,
	.cmd = mx_cmd
};

// vim: tabstop=4 shiftwidth=4 autoindent
