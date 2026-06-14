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

// ---- lenient reads: try keys in order, accept old/renamed spellings --------

// -----------------------------------------------------------------------
static const char * getstr_any(em400_cfg *cfg, const char *def, const char **keys)
{
	for (int i=0 ; keys[i] ; i++) {
		const char *v = cfg_getstr(cfg, keys[i], NULL);
		if (v) return v;
	}
	return def;
}

// -----------------------------------------------------------------------
static int getint_any(em400_cfg *cfg, int def, const char **keys)
{
	for (int i=0 ; keys[i] ; i++) {
		if (cfg_contains(cfg, keys[i])) return cfg_getint(cfg, keys[i], def);
	}
	return def;
}

// -----------------------------------------------------------------------
static bool getbool_any(em400_cfg *cfg, bool def, const char **keys)
{
	for (int i=0 ; keys[i] ; i++) {
		if (cfg_contains(cfg, keys[i])) return cfg_getbool(cfg, keys[i], def);
	}
	return def;
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
// ---- new format: per-machine sections with dotted keys ----------------
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
static int build_device_new(em400_cfg *cfg, const char *sec, int chnum, int devnum, struct em400_device_cfg *dev)
{
	const char *dev_type_name = cfg_fgetstr(cfg, "%s:dev.%i.%i.type", sec, chnum, devnum);
	if (!dev_type_name) {
		dev->type = EM400_DEV_NONE;
		return E_OK;
	}

	LOG(L_EM4H, "Configuring device %i.%i (%s)", chnum, devnum, dev_type_name);

	if (!strcasecmp(dev_type_name, "terminal")) {
		const int port = cfg_fgetint(cfg, "%s:dev.%i.%i.port", sec, chnum, devnum);
		if (port == -1) {
			return LOGERR("Device %i.%i: terminal needs TCP port to be set.", chnum, devnum);
		}
		int speed = cfg_fgetint(cfg, "%s:dev.%i.%i.speed", sec, chnum, devnum);
		if (speed == -1) {
			speed = 9600;
		}
		dev->type = EM400_DEV_TERMINAL;
		dev->terminal.port = port;
		dev->terminal.speed = speed;
	} else if (!strcasecmp(dev_type_name, "winchester")) {
		dev->type = EM400_DEV_WINCHESTER;
		dev->winchester.image = dup_str(cfg_fgetstr(cfg, "%s:dev.%i.%i.image", sec, chnum, devnum));
	} else if (!strcasecmp(dev_type_name, "floppy")) {
		dev->type = EM400_DEV_FLOP5;
	} else if (!strcasecmp(dev_type_name, "sp45de") || !strcasecmp(dev_type_name, "floppy8")) {
		dev->type = EM400_DEV_SP45DE;
		for (int slot=0 ; slot<EM400_SP45DE_SLOT_COUNT ; slot++) {
			const char *img = cfg_fgetstr(cfg, "%s:dev.%i.%i.slot.%i", sec, chnum, devnum, slot);
			if (!img) {
				img = cfg_fgetstr(cfg, "%s:dev.%i.%i.image_%i", sec, chnum, devnum, slot);
			}
			// empty placeholder (the writer emits "slot.N = ") means no media
			dev->sp45de.images[slot] = (img && *img) ? strdup(img) : NULL;
		}
	} else if (!strcasecmp(dev_type_name, "rtclock")) {
		dev->type = EM400_DEV_RTCLOCK;
		dev->rtclock.prom = dup_str(cfg_fgetstr(cfg, "%s:dev.%i.%i.prom", sec, chnum, devnum));
	} else {
		return LOGERR("Unknown device type: %s", dev_type_name);
	}

	return E_OK;
}

// -----------------------------------------------------------------------
static int build_io_new(em400_cfg *cfg, const char *sec, struct em400_machine_cfg *machine)
{
	for (int chnum=0 ; chnum<EM400_IO_MAX_CHAN ; chnum++) {
		struct em400_channel_cfg *chan = &machine->channel[chnum];
		chan->type = EM400_CHANNEL_NONE;
		for (int devnum=0 ; devnum<EM400_CHAN_MAX_DEV ; devnum++) {
			chan->device[devnum].type = EM400_DEV_NONE;
		}

		const char *ch_name = cfg_fgetstr(cfg, "%s:channel.%i", sec, chnum);
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
			if (build_device_new(cfg, sec, chnum, devnum, &chan->device[devnum]) != E_OK) {
				return LOGERR("Device %i:%i configuration error", chnum, devnum);
			}
		}
	}

	return E_OK;
}

// -----------------------------------------------------------------------
static int build_machine_new(em400_cfg *cfg, const char *sec, struct em400_machine_cfg *machine)
{
	*machine = (struct em400_machine_cfg) {
		.cpu = {
			.awp = cfg_fgetbool_def(cfg, CFG_DEFAULT_CPU_AWP, "%s:cpu.awp", sec),
			.mod = cfg_fgetbool_def(cfg, CFG_DEFAULT_CPU_MODIFICATIONS, "%s:cpu.modifications", sec),
			.user_io_illegal = cfg_fgetbool_def(cfg, CFG_DEFAULT_CPU_IO_USER_ILLEGAL, "%s:cpu.user_io_illegal", sec),
			.nomem_stop = cfg_fgetbool_def(cfg, CFG_DEFAULT_CPU_STOP_ON_NOMEM, "%s:cpu.stop_on_nomem", sec),
			.clock_period_ms = cfg_fgetint_def(cfg, CFG_DEFAULT_CPU_CLOCK_PERIOD_MS, "%s:cpu.clock_period", sec),
		},
		.mem = {
			.elwro_modules = cfg_fgetint_def(cfg, CFG_DEFAULT_MEMORY_ELWRO_MODULES, "%s:memory.elwro_modules", sec),
			.mega_modules = cfg_fgetint_def(cfg, CFG_DEFAULT_MEMORY_MEGA_MODULES, "%s:memory.mega_modules", sec),
			.os_segments = cfg_fgetint_def(cfg, CFG_DEFAULT_MEMORY_HARDWIRED_SEGMENTS, "%s:memory.hardwired_segments", sec),
			.mega_prom_image = dup_str(cfg_fgetstr(cfg, "%s:memory.mega_prom", sec)),
			.preload_image = dup_str(cfg_fgetstr(cfg, "%s:memory.preload", sec)),
		},
	};

	return build_io_new(cfg, sec, machine);
}

// -----------------------------------------------------------------------
static bool has_machine_sections(em400_cfg *cfg)
{
	int nsec = iniparser_getnsec(cfg);
	for (int i=0 ; i<nsec ; i++) {
		const char *sec = iniparser_getsecname(cfg, i);
		if (sec && !strncmp(sec, "machine.", 8)) return true;
	}
	return false;
}

// -----------------------------------------------------------------------
static int build_new(em400_cfg *cfg)
{
	int nsec = iniparser_getnsec(cfg);
	for (int i=0 ; i<nsec ; i++) {
		const char *sec = iniparser_getsecname(cfg, i);
		if (!sec || strncmp(sec, "machine.", 8)) continue;

		const char *id = sec + 8;
		const char *name = cfg_fgetstr(cfg, "%s:name", sec);
		LOG(L_EM4H, "Configuring machine '%s' (%s)", id, name ? name : "unnamed");
		struct appcfg_machine *m = appcfg_machine_add(&appcfg, id, name);
		if (!m) {
			return LOGERR("Failed to allocate machine configuration: %s", id);
		}
		if (build_machine_new(cfg, sec, &m->cfg) != E_OK) {
			return LOGERR("Failed to build configuration for machine: %s", id);
		}
	}

	// unresolvable/missing active id is tolerated: appcfg_active_machine falls back to the first
	appcfg.active_id = dup_str(cfg_getstr(cfg, "general:machine", NULL));
	LOG(L_EM4H, "Configured %i machine(s), active: %s", appcfg.n_machines,
		appcfg.active_id ? appcfg.active_id : "(unset, will use the first one)");

	return E_OK;
}

// -----------------------------------------------------------------------
// host/ui/log read leniently in both formats: try the new key, then the
// legacy spelling, so configs from 0.4.0 onward still load
static void build_host(em400_cfg *cfg)
{
	appcfg.host = (struct em400_host_cfg) {
		.emu = {
			.speed_real = getbool_any(cfg, CFG_DEFAULT_CPU_SPEED_REAL,
				(const char *[]){"general:speed_real", "cpu:speed_real", NULL}),
			.emulation_quantum_us = getint_any(cfg, CFG_DEFAULT_CPU_EMULATION_QUANTUM_US,
				(const char *[]){"general:emulation_quantum_us", "cpu:emulation_quantum_us", "cpu:throttle_granularity", NULL}),
		},
		.sound = {
			.enabled = cfg_getbool(cfg, "sound:enabled", CFG_DEFAULT_SOUND_ENABLED),
			.buffer_len = getint_any(cfg, CFG_DEFAULT_SOUND_BUFFER_LEN,
				(const char *[]){"sound:buffer_len", "sound:buffer", NULL}),
			.volume = cfg_getint(cfg, "sound:volume", CFG_DEFAULT_SOUND_VOLUME),
			.sample_rate = cfg_getint(cfg, "sound:rate", CFG_DEFAULT_SOUND_RATE),
			.latency = cfg_getint(cfg, "sound:latency", CFG_DEFAULT_SOUND_LATENCY),
			.backend = dup_str(getstr_any(cfg, CFG_DEFAULT_SOUND_BACKEND,
				(const char *[]){"sound:backend", "sound:driver", NULL})),
			.device = dup_str(getstr_any(cfg, CFG_DEFAULT_SOUND_DEVICE,
				(const char *[]){"sound:device", "sound:output", NULL})),
		},
	};

	appcfg.ui = dup_str(getstr_any(cfg, NULL, (const char *[]){"general:ui", "ui:interface", NULL}));

	appcfg.log = (struct appcfg_log) {
		.enabled = cfg_getbool(cfg, "log:enabled", CFG_DEFAULT_LOG_ENABLED),
		.components = dup_str(cfg_getstr(cfg, "log:components", CFG_DEFAULT_LOG_COMPONENTS)),
		.file = dup_str(cfg_getstr(cfg, "log:file", CFG_DEFAULT_LOG_FILE)),
		.line_buffered = cfg_getbool(cfg, "log:line_buffered", CFG_DEFAULT_LOG_LINE_BUFFERED),
	};

	LOG(L_EM4H, "Host config: ui=%s, speed_real=%s, emulation_quantum=%ius, sound=%s",
		appcfg.ui ? appcfg.ui : "(default)",
		appcfg.host.emu.speed_real ? "true" : "false",
		appcfg.host.emu.emulation_quantum_us,
		appcfg.host.sound.enabled ? "enabled" : "disabled");
}

// -----------------------------------------------------------------------
int appcfg_build_from_ini(em400_cfg *cfg)
{
	build_host(cfg);

	if (has_machine_sections(cfg)) {
		LOG(L_EM4H, "Reading new-format configuration (per-machine sections)");
		return build_new(cfg);
	}

	// legacy format describes exactly one unnamed machine
	LOG(L_EM4H, "Reading legacy-format configuration, importing it as machine '%s'", APPCFG_IMPORTED_MACHINE_ID);
	struct appcfg_machine *m = appcfg_machine_add(&appcfg, APPCFG_IMPORTED_MACHINE_ID, "Imported configuration");
	if (!m) {
		return LOGERR("Failed to allocate machine configuration");
	}
	if (build_machine(cfg, &m->cfg) != E_OK) {
		return LOGERR("Failed to build EM400 I/O configuration");
	}
	appcfg.active_id = dup_str(APPCFG_IMPORTED_MACHINE_ID);

	return E_OK;
}

// -----------------------------------------------------------------------
static const char * bstr(bool b)
{
	return b ? "true" : "false";
}

// -----------------------------------------------------------------------
static const char * channel_type_name(enum em400_channel_types type)
{
	switch (type) {
		case EM400_CHANNEL_CHAR: return "char";
		case EM400_CHANNEL_MULTIX: return "multix";
		case EM400_CHANNEL_IOTESTER: return "iotester";
		default: return NULL;
	}
}

// -----------------------------------------------------------------------
static const char * device_type_name(enum em400_device_types type)
{
	switch (type) {
		case EM400_DEV_TERMINAL: return "terminal";
		case EM400_DEV_WINCHESTER: return "winchester";
		case EM400_DEV_FLOP5: return "floppy";
		case EM400_DEV_SP45DE: return "sp45de";
		case EM400_DEV_RTCLOCK: return "rtclock";
		default: return NULL;
	}
}

// -----------------------------------------------------------------------
static void write_device(FILE *f, int chnum, int devnum, const struct em400_device_cfg *dev)
{
	const char *type = device_type_name(dev->type);
	if (!type) return;

	fprintf(f, "dev.%i.%i.type = %s\n", chnum, devnum, type);

	switch (dev->type) {
		case EM400_DEV_TERMINAL:
			fprintf(f, "dev.%i.%i.port = %i\n", chnum, devnum, dev->terminal.port);
			if (dev->terminal.speed != 9600) {
				fprintf(f, "dev.%i.%i.speed = %i\n", chnum, devnum, dev->terminal.speed);
			}
			break;
		case EM400_DEV_WINCHESTER:
			if (dev->winchester.image) {
				fprintf(f, "dev.%i.%i.image = %s\n", chnum, devnum, dev->winchester.image);
			}
			break;
		case EM400_DEV_SP45DE:
			for (int slot=0 ; slot<EM400_SP45DE_SLOT_COUNT ; slot++) {
				fprintf(f, "dev.%i.%i.slot.%i = %s\n", chnum, devnum, slot,
					dev->sp45de.images[slot] ? dev->sp45de.images[slot] : "");
			}
			break;
		case EM400_DEV_RTCLOCK:
			if (dev->rtclock.prom) {
				fprintf(f, "dev.%i.%i.prom = %s\n", chnum, devnum, dev->rtclock.prom);
			}
			break;
		default:
			break;
	}
}

// -----------------------------------------------------------------------
static void write_machine(FILE *f, const struct appcfg_machine *m)
{
	const struct em400_machine_cfg *cfg = &m->cfg;

	fprintf(f, "\n[machine.%s]\n", m->id);
	if (m->name) {
		fprintf(f, "name = %s\n", m->name);
	}

	if (cfg->cpu.clock_period_ms != CFG_DEFAULT_CPU_CLOCK_PERIOD_MS) {
		fprintf(f, "cpu.clock_period = %i\n", cfg->cpu.clock_period_ms);
	}
	if (cfg->cpu.nomem_stop != CFG_DEFAULT_CPU_STOP_ON_NOMEM) {
		fprintf(f, "cpu.stop_on_nomem = %s\n", bstr(cfg->cpu.nomem_stop));
	}
	if (cfg->cpu.user_io_illegal != CFG_DEFAULT_CPU_IO_USER_ILLEGAL) {
		fprintf(f, "cpu.user_io_illegal = %s\n", bstr(cfg->cpu.user_io_illegal));
	}
	if (cfg->cpu.awp != CFG_DEFAULT_CPU_AWP) {
		fprintf(f, "cpu.awp = %s\n", bstr(cfg->cpu.awp));
	}
	if (cfg->cpu.mod != CFG_DEFAULT_CPU_MODIFICATIONS) {
		fprintf(f, "cpu.modifications = %s\n", bstr(cfg->cpu.mod));
	}

	if (cfg->mem.elwro_modules != CFG_DEFAULT_MEMORY_ELWRO_MODULES) {
		fprintf(f, "memory.elwro_modules = %i\n", cfg->mem.elwro_modules);
	}
	if (cfg->mem.mega_modules != CFG_DEFAULT_MEMORY_MEGA_MODULES) {
		fprintf(f, "memory.mega_modules = %i\n", cfg->mem.mega_modules);
	}
	if (cfg->mem.os_segments != CFG_DEFAULT_MEMORY_HARDWIRED_SEGMENTS) {
		fprintf(f, "memory.hardwired_segments = %i\n", cfg->mem.os_segments);
	}
	if (cfg->mem.mega_prom_image) {
		fprintf(f, "memory.mega_prom = %s\n", cfg->mem.mega_prom_image);
	}
	if (cfg->mem.preload_image) {
		fprintf(f, "memory.preload = %s\n", cfg->mem.preload_image);
	}

	for (int chnum=0 ; chnum<EM400_IO_MAX_CHAN ; chnum++) {
		const struct em400_channel_cfg *chan = &cfg->channel[chnum];
		const char *ch_type = channel_type_name(chan->type);
		if (!ch_type) continue;

		fprintf(f, "channel.%i = %s\n", chnum, ch_type);
		for (int devnum=0 ; devnum<EM400_CHAN_MAX_DEV ; devnum++) {
			write_device(f, chnum, devnum, &chan->device[devnum]);
		}
	}
}

// -----------------------------------------------------------------------
int appcfg_write(const struct appcfg *c, const char *path)
{
	LOG(L_EM4H, "Writing configuration (%i machine(s)) to %s", c->n_machines, path);

	FILE *f = fopen(path, "w");
	if (!f) {
		return LOGERR("Cannot open config file for writing: %s", path);
	}

	fprintf(f, "[general]\n");
	if (c->ui) {
		fprintf(f, "ui = %s\n", c->ui);
	}
	if (c->active_id) {
		fprintf(f, "machine = %s\n", c->active_id);
	}
	fprintf(f, "speed_real = %s\n", bstr(c->host.emu.speed_real));
	fprintf(f, "emulation_quantum_us = %i\n", c->host.emu.emulation_quantum_us);

	fprintf(f, "\n[log]\n");
	fprintf(f, "enabled = %s\n", bstr(c->log.enabled));
	if (c->log.file) {
		fprintf(f, "file = %s\n", c->log.file);
	}
	if (c->log.components) {
		fprintf(f, "components = %s\n", c->log.components);
	}
	fprintf(f, "line_buffered = %s\n", bstr(c->log.line_buffered));

	const struct em400_sound_cfg *snd = &c->host.sound;
	fprintf(f, "\n[sound]\n");
	fprintf(f, "enabled = %s\n", bstr(snd->enabled));
	fprintf(f, "rate = %i\n", snd->sample_rate);
	fprintf(f, "buffer_len = %i\n", snd->buffer_len);
	fprintf(f, "latency = %i\n", snd->latency);
	fprintf(f, "volume = %i\n", snd->volume);
	if (snd->backend && strcmp(snd->backend, CFG_DEFAULT_SOUND_BACKEND)) {
		fprintf(f, "backend = %s\n", snd->backend);
	}
	if (snd->device) {
		fprintf(f, "device = %s\n", snd->device);
	}

	for (int i=0 ; i<c->n_machines ; i++) {
		write_machine(f, &c->machines[i]);
	}

	fclose(f);
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
	free(appcfg.ui);
	free(appcfg.log.components);
	free(appcfg.log.file);

	free((void *) appcfg.host.sound.backend);
	free((void *) appcfg.host.sound.device);

	appcfg = (struct appcfg) {0};
}

// vim: tabstop=4 shiftwidth=4 autoindent
