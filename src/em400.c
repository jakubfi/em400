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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "ui/ui.h"
#include "cp/cp.h"
#include "cpu/cpu.h"
#include "mem/mem.h"
#include "cpu/interrupts.h"
#include "cpu/clock.h"
#include "io/io.h"
#include "io/chan.h"
#include "io/dev2/dev2.h"
#include "io/dev2/terminal.h"
#include "io/dev2/sp45de.h"

#include "cfg.h"

#include "log.h"

struct ui *ui;

struct io_chan_types {
	const char *name;
	unsigned type;
} io_chan_types[] = {
	{"multix", CHAN_MULTIX},
	{"char", CHAN_CHAR},
	{"iotester", CHAN_IOTESTER},
	{NULL, 0}
};

// -----------------------------------------------------------------------
static struct io_chan_types * find_chan_type(const char *name)
{
	struct io_chan_types *chmap = io_chan_types;
	while (chmap && chmap->name) {
		if (!strcasecmp(name, chmap->name)) {
			return chmap;
		}
		chmap++;
	}
	return NULL;
}

// -----------------------------------------------------------------------
int em400_channels_init(em400_cfg *cfg)
{
	for (int i=0 ; i<16 ; i++) {
		const char *ch_name = cfg_fgetstr(cfg, "io:channel_%i", i);
		if (!ch_name) continue;

		LOG(L_EM4H, "Initializing I/O channel %i: %s", i, ch_name);

		struct io_chan_types *chmap = find_chan_type(ch_name);
		if (!chmap) {
			return LOGERR("Unknown channel type: %s", ch_name);
		}

		em400_io_channel_init(i, chmap->type);
	}

	return E_OK;
}

// -----------------------------------------------------------------------
int em400_devices_init(em400_cfg *cfg)
{
	for (int chnum=0 ; chnum<16 ; chnum++) {
		if (!io_channel_present(chnum)) continue;
		LOG(L_EM4H, "Processing devices in channel %i", chnum);
		for (int devnum=0 ; devnum<32 ; devnum++) {
			const char *dev_type_name = cfg_fgetstr(cfg, "dev%i.%i:type", chnum, devnum);
			if (!dev_type_name) continue;
			LOG(L_EM4H, "Initializing device %i:%i (%s)", chnum, devnum, dev_type_name);
			if (!strcasecmp(dev_type_name, "terminal")) {
				const char *transport = cfg_fgetstr(cfg, "dev%i.%i:transport", chnum, devnum);
				if (transport && strcasecmp(transport, "tcp")) {
					return LOGERR("Fake terminal only supports TCP transport type");
				}
				const int port = cfg_fgetint(cfg, "dev%i.%i:port", chnum, devnum);
				if (port == -1) {
					return LOGERR("Device %i.%i: Fake TCP terminal needs port to be set.", chnum, devnum);
				}
				em400_dev_terminal_fake_init(chnum, devnum, port);
			} else if (!strcasecmp(dev_type_name, "terminal2")) {
				const char *transport = cfg_fgetstr(cfg, "dev%i.%i:transport", chnum, devnum);
				if (transport && strcasecmp(transport, "tcp")) {
					return LOGERR("Terminal only supports TCP transport type");
				}
				const int port = cfg_fgetint(cfg, "dev%i.%i:port", chnum, devnum);
				if (port == -1) {
					return LOGERR("Device %i.%i: TCP terminal needs port to be set.", chnum, devnum);
				}
				int speed = cfg_fgetint(cfg, "dev%i.%i:speed", chnum, devnum);
				if (speed == -1) {
					LOG(L_EM4H, "Device %i.%i: terminal speed not set, defaulting to 9600", chnum, devnum);
					speed = 9600;
				}
				em400_dev_terminal_init(chnum, devnum, port, speed);
			} else if (!strcasecmp(dev_type_name, "winchester")) {
				const char *image = cfg_fgetstr(cfg, "dev%i.%i:image", chnum, devnum);
				em400_dev_winchester_init(chnum, devnum, image);
			} else if (!strcasecmp(dev_type_name, "floppy")) {

			} else if (!strcasecmp(dev_type_name, "floppy8")) {
				const char *images[4];
				images[0] = cfg_fgetstr(cfg, "dev%i.%i:image_0", chnum, devnum);
				images[1] = cfg_fgetstr(cfg, "dev%i.%i:image_1", chnum, devnum);
				images[2] = cfg_fgetstr(cfg, "dev%i.%i:image_2", chnum, devnum);
				images[3] = cfg_fgetstr(cfg, "dev%i.%i:image_3", chnum, devnum);
				em400_dev_sp45de_init(chnum, devnum, images);
			} else {
				LOGERR("Unknown device type: %s", dev_type_name);
			}
		}
	}

	return E_OK;
}

// -----------------------------------------------------------------------
int em400_top_init(em400_cfg *cfg)
{

	const char *log_file_name = cfg_getstr(cfg, "log:file", CFG_DEFAULT_LOG_FILE);
	em400_log_buf_type_t log_buf_type =
		cfg_getbool(cfg, "log:line_buffered", CFG_DEFAULT_LOG_LINE_BUFFERED)
		? EM400_LOG_LINE_BUFFERED
		: EM400_LOG_FULL_BUFFERED;
	const char *log_components = cfg_getstr(cfg, "log:components", CFG_DEFAULT_LOG_COMPONENTS);
	bool log_enabled = cfg_getbool(cfg, "log:enabled", CFG_DEFAULT_LOG_ENABLED);

	if (log_init(log_file_name, log_buf_type) != E_OK) {
		return LOGERR("Failed to initialize logging.");
	}
	if (log_enabled) {
		if (log_enable() != E_OK) {
			return LOGERR("Failed to enable logging.");
		}
	}
	if (log_setup_components(log_components)) {
		return LOGERR("Failed to set which components to log");
	}

	struct em400_cfg_cpu cpu_cfg = {
		.awp = cfg_getbool(cfg, "cpu:awp", CFG_DEFAULT_CPU_AWP),
		.mod = cfg_getbool(cfg, "cpu:modifications", CFG_DEFAULT_CPU_MODIFICATIONS),
		.user_io_illegal = cfg_getbool(cfg, "cpu:user_io_illegal", CFG_DEFAULT_CPU_IO_USER_ILLEGAL),
		.nomem_stop = cfg_getbool(cfg, "cpu:stop_on_nomem", CFG_DEFAULT_CPU_STOP_ON_NOMEM),
		.speed_real = cfg_getbool(cfg, "cpu:speed_real", CFG_DEFAULT_CPU_SPEED_REAL),
		.throttle_granularity = 1000 * cfg_getint(cfg, "cpu:throttle_granularity", CFG_DEFAULT_CPU_THROTTLE_GRANULARITY),
		.clock_period = cfg_getint(cfg, "cpu:clock_period", CFG_DEFAULT_CPU_CLOCK_PERIOD),
	};

	struct em400_cfg_buzzer buzzer_cfg = {
		.enabled = cfg_getbool(cfg, "sound:enabled", CFG_DEFAULT_SOUND_ENABLED),
		.buffer_len = cfg_getint(cfg, "sound:buffer_len", CFG_DEFAULT_SOUND_BUFFER_LEN),
		.volume = cfg_getint(cfg, "sound:volume", CFG_DEFAULT_SOUND_VOLUME),
		.sample_rate = cfg_getint(cfg, "sound:rate", CFG_DEFAULT_SOUND_RATE),
		.driver = cfg_getstr(cfg, "sound:driver", CFG_DEFAULT_SOUND_DRIVER),
		.output = cfg_getstr(cfg, "sound:output", CFG_DEFAULT_SOUND_OUTPUT),
		.latency = cfg_getint(cfg, "sound:latency", CFG_DEFAULT_SOUND_LATENCY),
	};

	struct em400_cfg_mem mem_cfg = {
		.elwro_modules = cfg_getint(cfg, "memory:elwro_modules", CFG_DEFAULT_MEMORY_ELWRO_MODULES),
		.mega_modules = cfg_getint(cfg, "memory:mega_modules", CFG_DEFAULT_MEMORY_MEGA_MODULES),
		.os_segments = cfg_getint(cfg, "memory:hardwired_segments", CFG_DEFAULT_MEMORY_HARDWIRED_SEGMENTS),
		.mega_prom_image = cfg_getstr(cfg, "memory:mega_prom", CFG_DEFAULT_MEMORY_MEGA_PROM),
	};

	if (em400_init(&mem_cfg, &cpu_cfg, &buzzer_cfg) != E_OK) return LOGERR("Failed to initialize EM400 library.");
	em400_channels_init(cfg);
	em400_devices_init(cfg);

	if (!(ui = ui_create(cfg))) return LOGERR("Failed to initialize UI.");

	return E_OK;
}

// -----------------------------------------------------------------------
void em400_top_shutdown()
{
	ui_shutdown(ui);
	em400_destroy();
	log_shutdown();
}

// -----------------------------------------------------------------------
int em400_preload_program(const char *program_name)
{
	if (!program_name) {
		return E_OK;
	}

	FILE *f = fopen(program_name, "rb");
	if (!f) {
		return LOGERR("Failed to open program file: \"%s\".", program_name);
	}

	bool res = em400_load_os_image(f);
	fclose(f);
	if (!res) {
		return LOGERR("Failed to preload program file: \"%s\".", program_name);
	} else {
		LOG(L_EM4H, "OS memory block preloaded with \"%s\", %i words", program_name, res);
	}

	return E_OK;
}

// -----------------------------------------------------------------------
void em400_usage()
{
	fprintf(stdout, "EM400 version %s\n", em400_version());
	fprintf(stdout,
		"Usage: em400 [option] ...\n"
		"\n"
		"Options:\n"
		"   -h               : Display help\n"
		"   -c config        : Config file to use instead of the default one (~/.em400/em400.cfg)\n"
		"   -p program       : Load program image into OS memory at address 0\n"
		"   -l cmp,cmp,...   : Enable logging for specified components. Available components:\n"
		"                      reg, mem, cpu, op, int, io, mx, px, cchar, cmem, term\n"
		"                      wnch, flop, pnch, pnrd, tape, crk5, em4h, all\n"
		"   -L               : Disable logging\n"
		"   -u ui            : User interface to use. Available UIs (first is the default):"
	);
	ui_print_uis(stdout);
	fprintf(stdout, "\n");
	fprintf(stdout,
		"   -O sec:key=value : Override configuration entry \"key\" in section [sec] with a specific value\n"
	);
}

// -----------------------------------------------------------------------
void em400_mkconfdir()
{
	const char *home = getenv("HOME");
	const char *cdir = "/.em400";
	char *conf_dirname = (char *) malloc(strlen(home) + strlen(cdir) + 1);
	if (conf_dirname) {
		sprintf(conf_dirname, "%s%s", home, cdir);
		mkdir(conf_dirname, 0700);
		free(conf_dirname);
	}
}

const char em400_cmdline_opts[] = "hc:p:l:Lu:O:";

// -----------------------------------------------------------------------
int em400_cmdline_1(int argc, char **argv, int *print_help, char **config)
{
	int option;

	while ((option = getopt(argc, argv, em400_cmdline_opts)) != -1) {
		switch (option) {
			case 'h':
				*print_help = 1;
				break;
			case 'c':
				*config = strdup(optarg);
				break;
            case 'p':
            case 'L':
            case 'l':
            case 'u':
			case 'O':
				break;
            default:
                return E_ERR;
		}
	}

	return E_OK;
}

// -----------------------------------------------------------------------
int em400_cmdline_2(em400_cfg *cfg, int argc, char **argv)
{
    int option;
    optind = 1; // reset to 1 so consecutive calls work
	char *key, *val, *colon;

    while ((option = getopt(argc, argv, em400_cmdline_opts)) != -1) {
        switch (option) {
            case 'h':
            case 'c':
                break;
            case 'p':
                cfg_set(cfg, "memory:preload", optarg);
                break;
            case 'L':
				cfg_set(cfg, "log:enabled", "false");
                break;
            case 'l':
				cfg_set(cfg, "log:enabled", "true");
				cfg_set(cfg, "log:components", optarg);
                break;
            case 'u':
                cfg_set(cfg, "ui:interface", optarg);
                break;
			case 'O':
				colon = strchr(optarg, ':');
				key = strtok(optarg, "=");
				val = strtok(NULL, "=");
				if (!colon || !key || !val) {
					return LOGERR("Syntax error for -O switch. Should be: -O section:key=value");
				}
				cfg_set(cfg, key, val);
				break;
            default:
                return E_ERR;
        }
    }

    return E_OK;
}

// -----------------------------------------------------------------------
// ---- MAIN -------------------------------------------------------------
// -----------------------------------------------------------------------
int main(int argc, char** argv)
{
	int return_code = -1;

	int print_help = 0;
	char *config = NULL;
	em400_cfg *cfg = NULL;

	em400_mkconfdir();

	if (em400_cmdline_1(argc, argv, &print_help, &config) != E_OK) {
		LOGERR("Failed to parse commandline arguments.");
		goto done;
	}

	if (print_help) {
		em400_usage();
		return_code = 0;
		goto done;
	}

	if (!config) {
		const char *home = getenv("HOME");
		const char *cfile = "/.em400/em400.ini";
		config = (char *) malloc(strlen(home) + strlen(cfile) + 1);
		if (!config) {
			LOGERR("Memory allocation error.");
			goto done;
		}
		sprintf(config, "%s%s", home, cfile);
	}

	cfg = cfg_load(config);
	if (!cfg) {
		LOGERR("Failed to load config file: \"%s\".", config);
		goto done;
	}

	// read the commandline again to build final configuration
	if (em400_cmdline_2(cfg, argc, argv) != E_OK) {
		LOGERR("Failed to parse commandline arguments.");
		goto done;
	}

	if (em400_top_init(cfg) != E_OK) {
		LOGERR("Failed to initialize EM400.");
		goto done;
	}

	em400_preload_program(cfg_getstr(cfg, "memory:preload", CFG_DEFAULT_MEMORY_PRELOAD));

	if (ui_run(ui) != E_OK) {
		LOGERR("Failed to start the UI: %s.", ui->drv->name);
		goto done;
	}

	cpu_loop();

	return_code = 0;

done:
	em400_top_shutdown();
	cfg_free(cfg);
	free(config);
	return return_code;
}

// vim: tabstop=4 shiftwidth=4 autoindent
