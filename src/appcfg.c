//  Copyright (c) 2026 Jakub Filipowicz <jakubf@gmail.com>
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

#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "appcfg.h"
#include "log.h"

struct appcfg appcfg;

// -----------------------------------------------------------------------
static char * dup_str(const char *s)
{
	return s ? strdup(s) : NULL;
}

// -----------------------------------------------------------------------
static void machine_free(struct em400_machine_cfg *machine)
{
	free((void *) machine->mem.mega_prom_image);
	free((void *) machine->mem.preload_image);

	for (int chnum=0 ; chnum<EM400_IO_MAX_CHAN ; chnum++) {
		struct em400_channel_cfg *chan = &machine->channel[chnum];
		for (int devnum=0 ; devnum<EM400_CHAN_MAX_DEV ; devnum++) {
			struct em400_device_cfg *dev = &chan->device[devnum];
			switch (dev->type) {
				case EM400_DEV_WINCHESTER:
					free((void *) dev->winchester.image);
					break;
				case EM400_DEV_RTCLOCK:
					free((void *) dev->rtclock.prom);
					break;
				case EM400_DEV_SP45DE:
					for (int slot=0 ; slot<EM400_SP45DE_SLOT_COUNT ; slot++) {
						free((void *) dev->sp45de.images[slot]);
					}
					break;
				default:
					break;
			}
		}
	}
}

// -----------------------------------------------------------------------
struct appcfg_machine * appcfg_machine_find(struct appcfg *c, const char *id)
{
	if (!id) return NULL;
	for (int i=0 ; i<c->n_machines ; i++) {
		if (!strcmp(c->machines[i].id, id)) {
			return &c->machines[i];
		}
	}
	return NULL;
}

// -----------------------------------------------------------------------
struct appcfg_machine * appcfg_machine_add(struct appcfg *c, const char *id, const char *name)
{
	if (c->n_machines == c->cap_machines) {
		int newcap = c->cap_machines ? c->cap_machines * 2 : 4;
		struct appcfg_machine *p = realloc(c->machines, newcap * sizeof(*p));
		if (!p) return NULL;
		c->machines = p;
		c->cap_machines = newcap;
	}

	struct appcfg_machine *m = &c->machines[c->n_machines];
	*m = (struct appcfg_machine) { .id = dup_str(id), .name = dup_str(name) };
	c->n_machines++;
	return m;
}

// -----------------------------------------------------------------------
void appcfg_machine_delete(struct appcfg *c, const char *id)
{
	struct appcfg_machine *m = appcfg_machine_find(c, id);
	if (!m) return;
	int i = m - c->machines;
	bool was_active = c->active_id && !strcmp(c->active_id, m->id);

	machine_free(&m->cfg);
	free(m->id);
	free(m->name);

	memmove(&c->machines[i], &c->machines[i+1], (c->n_machines - i - 1) * sizeof(*c->machines));
	c->n_machines--;

	if (was_active) {
		free(c->active_id);
		c->active_id = c->n_machines ? dup_str(c->machines[0].id) : NULL;
	}
}

// -----------------------------------------------------------------------
struct em400_machine_cfg * appcfg_active_machine(struct appcfg *c)
{
	struct appcfg_machine *m = appcfg_machine_find(c, c->active_id);
	if (!m) {
		m = c->n_machines ? &c->machines[0] : NULL;
	}
	return m ? &m->cfg : NULL;
}

// -----------------------------------------------------------------------
static int build_device(em400_cfg *cfg, int chnum, int devnum, struct em400_device_cfg *dev)
{
	const char *dev_type_name = cfg_fgetstr(cfg, "dev%i.%i:type", chnum, devnum);
	if (!dev_type_name) {
		dev->type = EM400_DEV_NONE;
		return E_OK;
	}

	LOG(L_EM4H, "Configuring device %i.%i (%s)", chnum, devnum, dev_type_name);

	if (!strcasecmp(dev_type_name, "terminal")) {
		// TODO: remove dead "transport"
		const char *transport = cfg_fgetstr(cfg, "dev%i.%i:transport", chnum, devnum);
		if (transport && strcasecmp(transport, "tcp")) {
			return LOGERR("Terminal only supports TCP transport type");
		}
		const int port = cfg_fgetint(cfg, "dev%i.%i:port", chnum, devnum);
		if (port == -1) {
			return LOGERR("Device %i.%i: terminal needs TCP port to be set.", chnum, devnum);
		}
		int speed = cfg_fgetint(cfg, "dev%i.%i:speed", chnum, devnum);
		if (speed == -1) {
			LOG(L_EM4H, "Device %i.%i: terminal speed not set, defaulting to 9600", chnum, devnum);
			speed = 9600;
		}
		dev->type = EM400_DEV_TERMINAL;
		dev->terminal.port = port;
		dev->terminal.speed = speed;
	} else if (!strcasecmp(dev_type_name, "winchester")) {
		dev->type = EM400_DEV_WINCHESTER;
		dev->winchester.image = dup_str(cfg_fgetstr(cfg, "dev%i.%i:image", chnum, devnum));
	} else if (!strcasecmp(dev_type_name, "floppy")) {
		dev->type = EM400_DEV_FLOP5;
	} else if (!strcasecmp(dev_type_name, "floppy8")) {
		dev->type = EM400_DEV_SP45DE;
		for (int slot=0 ; slot<EM400_SP45DE_SLOT_COUNT ; slot++) {
			dev->sp45de.images[slot] = dup_str(cfg_fgetstr(cfg, "dev%i.%i:image_%i", chnum, devnum, slot));
		}
	} else if (!strcasecmp(dev_type_name, "rtclock")) {
		dev->type = EM400_DEV_RTCLOCK;
		dev->rtclock.prom = dup_str(cfg_fgetstr(cfg, "dev%i.%i:prom", chnum, devnum));
	} else {
		return LOGERR("Unknown device type: %s", dev_type_name);
	}

	return E_OK;
}

// -----------------------------------------------------------------------
static int build_io(em400_cfg *cfg, struct em400_machine_cfg *machine)
{
	for (int chnum=0 ; chnum<EM400_IO_MAX_CHAN ; chnum++) {
		struct em400_channel_cfg *chan = &machine->channel[chnum];
		chan->type = EM400_CHANNEL_NONE;
		for (int devnum=0 ; devnum<EM400_CHAN_MAX_DEV ; devnum++) {
			chan->device[devnum].type = EM400_DEV_NONE;
		}

		const char *ch_name = cfg_fgetstr(cfg, "io:channel_%i", chnum);
		if (!ch_name) continue;

		LOG(L_EM4H, "Configuring I/O channel %i: %s", chnum, ch_name);

		if (!strcasecmp(ch_name, "char")) {
			chan->type = EM400_CHANNEL_CHAR;
		} else if (!strcasecmp(ch_name, "multix")) {
			chan->type = EM400_CHANNEL_MULTIX;
		} else if (!strcasecmp(ch_name, "iotester")) {
			chan->type = EM400_CHANNEL_IOTESTER;
		} else {
			return LOGERR("Unknown channel %i type: %s", chnum, ch_name);
		}

		for (int devnum=0 ; devnum<EM400_CHAN_MAX_DEV ; devnum++) {
			if (build_device(cfg, chnum, devnum, &chan->device[devnum]) != E_OK) {
				return LOGERR("Device %i:%i configuration error", chnum, devnum);
			}
		}
	}

	return E_OK;
}

// -----------------------------------------------------------------------
static int build_machine(em400_cfg *cfg, struct em400_machine_cfg *machine)
{
	*machine = (struct em400_machine_cfg) {
		.cpu = {
			.awp = cfg_getbool(cfg, "cpu:awp", CFG_DEFAULT_CPU_AWP),
			.mod = cfg_getbool(cfg, "cpu:modifications", CFG_DEFAULT_CPU_MODIFICATIONS),
			.user_io_illegal = cfg_getbool(cfg, "cpu:user_io_illegal", CFG_DEFAULT_CPU_IO_USER_ILLEGAL),
			.nomem_stop = cfg_getbool(cfg, "cpu:stop_on_nomem", CFG_DEFAULT_CPU_STOP_ON_NOMEM),
			.clock_period_ms = cfg_getint(cfg, "cpu:clock_period", CFG_DEFAULT_CPU_CLOCK_PERIOD_MS),
		},
		.mem = {
			.elwro_modules = cfg_getint(cfg, "memory:elwro_modules", CFG_DEFAULT_MEMORY_ELWRO_MODULES),
			.mega_modules = cfg_getint(cfg, "memory:mega_modules", CFG_DEFAULT_MEMORY_MEGA_MODULES),
			.os_segments = cfg_getint(cfg, "memory:hardwired_segments", CFG_DEFAULT_MEMORY_HARDWIRED_SEGMENTS),
			.mega_prom_image = dup_str(cfg_getstr(cfg, "memory:mega_prom", CFG_DEFAULT_MEMORY_MEGA_PROM)),
			.preload_image = dup_str(cfg_getstr(cfg, "memory:preload", CFG_DEFAULT_MEMORY_PRELOAD)),
		},
	};

	return build_io(cfg, machine);
}

// -----------------------------------------------------------------------
int appcfg_build_from_ini(em400_cfg *cfg)
{
	appcfg.host = (struct em400_host_cfg) {
		.emu = {
			.speed_real = cfg_getbool(cfg, "cpu:speed_real", CFG_DEFAULT_CPU_SPEED_REAL),
			.emulation_quantum_us = cfg_getint(cfg, "cpu:emulation_quantum_us", CFG_DEFAULT_CPU_EMULATION_QUANTUM_US),
		},
		.sound = {
			.enabled = cfg_getbool(cfg, "sound:enabled", CFG_DEFAULT_SOUND_ENABLED),
			.buffer_len = cfg_getint(cfg, "sound:buffer_len", CFG_DEFAULT_SOUND_BUFFER_LEN),
			.volume = cfg_getint(cfg, "sound:volume", CFG_DEFAULT_SOUND_VOLUME),
			.sample_rate = cfg_getint(cfg, "sound:rate", CFG_DEFAULT_SOUND_RATE),
			.latency = cfg_getint(cfg, "sound:latency", CFG_DEFAULT_SOUND_LATENCY),
			.backend = dup_str(cfg_getstr(cfg, "sound:backend", CFG_DEFAULT_SOUND_BACKEND)),
			.device = dup_str(cfg_getstr(cfg, "sound:device", CFG_DEFAULT_SOUND_DEVICE)),
		},
	};

	// old INI format describes exactly one machine, with no human-readable name
	struct appcfg_machine *m = appcfg_machine_add(&appcfg, APPCFG_DEFAULT_MACHINE_ID, NULL);
	if (!m) {
		return LOGERR("Failed to allocate machine configuration");
	}
	if (build_machine(cfg, &m->cfg) != E_OK) {
		return LOGERR("Failed to build EM400 I/O configuration");
	}
	appcfg.active_id = dup_str(APPCFG_DEFAULT_MACHINE_ID);

	return E_OK;
}

// -----------------------------------------------------------------------
void appcfg_free(void)
{
	for (int i=0 ; i<appcfg.n_machines ; i++) {
		machine_free(&appcfg.machines[i].cfg);
		free(appcfg.machines[i].id);
		free(appcfg.machines[i].name);
	}
	free(appcfg.machines);
	free(appcfg.active_id);

	free((void *) appcfg.host.sound.backend);
	free((void *) appcfg.host.sound.device);

	appcfg = (struct appcfg) {0};
}

// vim: tabstop=4 shiftwidth=4 autoindent
