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

#include "errors.h"
#include "log.h"
#include "io/mx.h"
#include "io/mx_cmds.h"
#include "io/mx_ev.h"
#include "io/mx_irq.h"
#include "io/chan.h"

// Real multix boots up in probably just under a second
// ~500ms (for ROM/RAM check) + ~185ms (for RAM cleanup).
// Here we need just a reasonable delay - big enough for
// OS scheduler to switch threads between MULTIX and CPU,
// so we don't finish MULTIX' job before switching to CPU thread.
#define RESET_WAIT_MSEC 150

enum mx_state {
	MX_STATE_RUN, MX_STATE_QUIT, MX_STATE_RESET
};

static void * mx_main(void *ptr);
void mx_shutdown(void *ch);

// -----------------------------------------------------------------------
// ---- CPU interface ----------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
void * mx_create(int num, struct cfg_unit *units)
{
	struct mx *multix = malloc(sizeof(struct mx));
	if (!multix) {
		gerr = E_ALLOC;
		goto cleanup;
	}

	multix->num = num;

	LOG(L_MX, 1, "MULTIX (ch:%i) Creating", multix->num);

	multix->state = MX_STATE_RESET;
	if (pthread_mutex_init(&multix->state_mutex, NULL)) {
		goto cleanup;
	}
	if (pthread_cond_init(&multix->state_cond, NULL)) {
		goto cleanup;
	}

	multix->reset_ack = 0;
	if (pthread_mutex_init(&multix->reset_ack_mutex, NULL)) {
		goto cleanup;
	}
	if (pthread_cond_init(&multix->reset_ack_cond, NULL)) {
		goto cleanup;
	}

	// create event queue
	multix->evq = mx_evq_create(-1);
	if (!multix->evq) {
		gerr = E_EVQ;
		goto cleanup;
	}

	// create interrupt system
	multix->irqq = mx_irqq_create(multix->num, MX_INTRQ_LEN);

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

	LOG(L_MX, 1, "MULTIX (ch:%i) Shutting down", multix->num);

	// notify main thread to quit
	pthread_mutex_lock(&multix->state_mutex);
	multix->state = MX_STATE_QUIT;
	pthread_mutex_unlock(&multix->state_mutex);

	mx_evq_disable(multix->evq);

	LOG(L_MX, 3, "MULTIX (ch:%i) Waiting for main thread to join", multix->num);
	pthread_join(multix->main_th, NULL);

	// destroy interrupt system
	mx_irqq_destroy(multix->irqq);

	// destroy event queue
	mx_evq_destroy(multix->evq);

	pthread_mutex_destroy(&multix->state_mutex);
	pthread_cond_destroy(&multix->state_cond);
	pthread_mutex_destroy(&multix->reset_ack_mutex);
	pthread_cond_destroy(&multix->reset_ack_cond);

	int num = multix->num;

	free(multix);

	LOG(L_MX, 3, "MULTIX (ch:%i) Shutdown complete", num);
}

// -----------------------------------------------------------------------
void mx_reset(void *ch)
{
	struct mx *multix = (struct mx *) ch;

	LOG(L_MX, 2, "MULTIX (ch:%i) Initiating reset", multix->num);

	// set multix state to "reset"
	pthread_mutex_lock(&multix->state_mutex);
	if (multix->state != MX_STATE_QUIT) {
		multix->state = MX_STATE_RESET;
	}
	pthread_mutex_unlock(&multix->state_mutex);

	// disable event queue
	// evq_dequeue() will return and further commands will fail
	mx_evq_disable(multix->evq);

	// wait for reset to really kick in
	pthread_mutex_lock(&multix->reset_ack_mutex);
	while (!multix->reset_ack) {
		LOG(L_MX, 3, "MULTIX (ch:%i) Waiting for reset to kick in", multix->num);
		pthread_cond_wait(&multix->reset_ack_cond, &multix->reset_ack_mutex);
	}
	pthread_mutex_unlock(&multix->reset_ack_mutex);
	LOG(L_MX, 2, "MULTIX (ch:%i) Reset initiated", multix->num);
}

// -----------------------------------------------------------------------
int mx_cmd(void *ch, int dir, uint16_t n_arg, uint16_t *r_arg)
{
	struct mx *multix = (struct mx *) ch;

	unsigned cmd		= ((n_arg & 0b1110000000000000) >> 13) | ((dir&1) << 3);
	unsigned chan_cmd	=  (n_arg & 0b0001100000000000) >> 11;
	unsigned llinen		=  (n_arg & 0b0001111111100000) >> 5;

	// channel commands
	if (cmd == MX_CMD_CHAN) {
		LOG(L_MX, 2, "MULTIX (ch:%i) incomming channel command %i: %s", multix->num, chan_cmd, mx_chan_cmd_names[chan_cmd]);

		switch (chan_cmd) {
			case MX_CMD_INTSPEC:
				// get intspec (always there, even if invalid)
				*r_arg = mx_irqq_get_intspec(multix->irqq);
				// notify main thread that IRQ has been received byu CPU
				mx_evq_enqueue(multix->evq, mx_ev_simple(MX_EV_INT_RECVD), MX_EVQ_F_WAIT);
				return IO_OK;
			case MX_CMD_EXISTS:
				// 'exists' does nothing, just returns OK
				return IO_OK;
			case MX_CMD_RESET:
				// initiate reset and return OK (actual reset is done in main thread)
				mx_reset(multix);
				return IO_OK;
			default:
				// handle other commands (only MX_CMD_INVALID in fact) as illegal in main thread
				if (mx_evq_enqueue(multix->evq, mx_ev_cmd(cmd, 0, 0), MX_EVQ_F_TRY) <= 0) {
					LOG(L_MX, 2, "MULTIX (ch:%i) ENGAGED", multix->num);
					return IO_EN;
				} else {
					return IO_OK;
				}
		}
	// handle general and line commands in main thread
	} else {
		LOG(L_MX, 2, "MULTIX (ch:%i line:%i) incomming general/line command %i: %s", multix->num, llinen, cmd, mx_cmd_names[cmd]);
		if (mx_evq_enqueue(multix->evq, mx_ev_cmd(cmd, llinen, *r_arg), MX_EVQ_F_TRY) <= 0) {
			LOG(L_MX, 2, "MULTIX (ch:%i) ENGAGED", multix->num);
			return IO_EN;
		} else {
			return IO_OK;
		}
	}

}

// -----------------------------------------------------------------------
struct chan_drv mx_chan_driver = {
	.name = "multix",
	.create = mx_create,
	.shutdown = mx_shutdown,
	.reset = mx_reset,
	.cmd = mx_cmd
};

// -----------------------------------------------------------------------
// ---- main -------------------------------------------------------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
static int mx_init(struct mx *multix)
{
	LOG(L_MX, 2, "MULTIX (ch:%i) Initializing", multix->num);

	int ret;

	pthread_mutex_lock(&multix->state_mutex);

	// notify CPU thread that reset has been initiated
	// (i.e. MULTIX won't send anything to CPU from now on)
	pthread_mutex_lock(&multix->reset_ack_mutex);
	multix->reset_ack = 1;
	pthread_cond_signal(&multix->reset_ack_cond);
	pthread_mutex_unlock(&multix->reset_ack_mutex);

	// loop while state is 'reset'
	// if another reset comes in during reset loop, we need to restart reset loop
	while (multix->state == MX_STATE_RESET) {
		LOG(L_MX, 3, "MULTIX (ch:%i) Initialization delay: %i ms", multix->num, RESET_WAIT_MSEC);
		// we set state to 'run' here - if another reset is initiated in cpu thread, state changes
		multix->state = MX_STATE_RUN;
		struct timespec abstime;
		clock_gettime(CLOCK_REALTIME, &abstime);
		long new_nsec = abstime.tv_nsec + RESET_WAIT_MSEC * 1000000L;
		abstime.tv_sec += new_nsec / 1000000000L;
		abstime.tv_nsec = new_nsec % 1000000000L;

		// wait on condition for RESET_WAIT_MSEC ms
		if (pthread_cond_timedwait(&multix->state_cond, &multix->state_mutex, &abstime)) {
			LOG(L_MX, 3, "MULTIX (ch:%i) reset wait interrupted", multix->num);
		} else {
			LOG(L_MX, 3, "MULTIX (ch:%i) reset wait done", multix->num);
		}
	}

	if (multix->state == MX_STATE_QUIT) {
		LOG(L_MX, 3, "MULTIX (ch:%i) Quit received during initialization", multix->num);
		ret = -1;
	} else {
		mx_evq_clear(multix->evq);
		mx_irqq_clear(multix->irqq);
		mx_irqq_enqueue(multix->irqq, MX_IRQ_IWYZE, 0);
		mx_evq_enable(multix->evq);
		ret = 0;

		// clear reset ack flag
		pthread_mutex_lock(&multix->reset_ack_mutex);
		multix->reset_ack = 0;
		pthread_cond_signal(&multix->reset_ack_cond);
		pthread_mutex_unlock(&multix->reset_ack_mutex);

		LOG(L_MX, 2, "MULTIX (ch:%i) Initialization done", multix->num);
	}

	pthread_mutex_unlock(&multix->state_mutex);

	return ret;
}

// -----------------------------------------------------------------------
static void mx_ev_handle_cmd(struct mx *multix, struct mx_ev *ev)
{
	switch (ev->cmd) {
		case MX_CMD_ERR_0:
			mx_irqq_enqueue(multix->irqq, MX_IRQ_IEPS0, 0);
			break;
		case MX_CMD_TEST:
			break;
		case MX_CMD_ATTACH:
			break;
		case MX_CMD_STATUS:
			break;
		case MX_CMD_TRANSMIT:
			break;
		case MX_CMD_SETCFG:
			break;
		case MX_CMD_ERR_6:
			mx_irqq_enqueue(multix->irqq, MX_IRQ_IEPS6, 0);
			break;
		case MX_CMD_ERR_7:
			mx_irqq_enqueue(multix->irqq, MX_IRQ_IEPS7, 0);
			break;
		case MX_CMD_ERR_8:
			mx_irqq_enqueue(multix->irqq, MX_IRQ_IEPS8, 0);
			break;
		case MX_CMD_INTRQ:
			break;
		case MX_CMD_DETACH:
			break;
		case MX_CMD_ABORT:
			break;
		case MX_CMD_ERR_C:
			mx_irqq_enqueue(multix->irqq, MX_IRQ_IEPSC, 0);
			break;
		case MX_CMD_ERR_D:
			mx_irqq_enqueue(multix->irqq, MX_IRQ_IEPSD, 0);
			break;
		case MX_CMD_ERR_E:
			mx_irqq_enqueue(multix->irqq, MX_IRQ_IEPSE, 0);
			break;
		case MX_CMD_ERR_F:
			mx_irqq_enqueue(multix->irqq, MX_IRQ_IEPSF, 0);
			break;
		default:
			LOG(L_MX, 1, "MULTIX (ch:%i) Unknown general/line command: %i - ignored", multix->num, ev->cmd);
			break;
	}
}

// -----------------------------------------------------------------------
static void mx_ev_handle(struct mx *multix, struct mx_ev *ev)
{
	if (ev->type == MX_EV_INT_RECVD) {
		mx_irqq_advance(multix->irqq);
	} else if (ev->type == MX_EV_CMD) {
		mx_ev_handle_cmd(multix, ev);
	} else {
		LOG(L_MX, 1, "MULTIX (ch:%i) Unknown event type: %i", multix->num, ev->type);
	}
}

// -----------------------------------------------------------------------
static void * mx_main(void *ptr)
{
	struct mx *multix = ptr;
	struct mx_ev *ev;

	LOG(L_MX, 3, "MULTIX (ch:%i) Starting main loop thread", multix->num);

	// initialize MX if 'reset' or, break the loop if 'quit'
	while (!mx_init(multix)) {
		// wait for non-empty event
		// if event is empty, that means state is 'quit' or 'reset'
		while ((ev = mx_evq_dequeue(multix->evq, MX_EVQ_F_WAIT))) {
			LOG(L_MX, 2, "MULTIX (ch:%i) Got event: %i (%s)", multix->num, ev->type, mx_event_names[ev->type]);
			mx_ev_handle(multix, ev);
			mx_ev_delete(ev);
			// TODO: run task manager
		}
		LOG(L_MX, 3, "MULTIX (ch:%i) Got empty event and left event queue", multix->num);
	}
	LOG(L_MX, 3, "MULTIX (ch:%i) exiting main loop thread", multix->num);

	pthread_exit(NULL);
}

// vim: tabstop=4 shiftwidth=4 autoindent
