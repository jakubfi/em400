//  Copyright (c) 2020 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef _CFG_H_
#define _CFG_H_

#include "external/iniparser/iniparser.h"
#include "cfg.h"

typedef dictionary em400_cfg;

#define CFG_DEFAULT_LOG_FILE "em400.log"
#define CFG_DEFAULT_LOG_COMPONENTS "em4h"
#define CFG_DEFAULT_LOG_LINE_BUFFERED 1
#define CFG_DEFAULT_LOG_ENABLED 0

#define CFG_DEFAULT_CPU_MODIFICATIONS 0
#define CFG_DEFAULT_CPU_FPGA 0
#define CFG_DEFAULT_CPU_AWP 1
#define CFG_DEFAULT_CPU_KB 0
#define CFG_DEFAULT_CPU_IO_USER_ILLEGAL 1
#define CFG_DEFAULT_CPU_STOP_ON_NOMEM 1
#define CFG_DEFAULT_CPU_SPEED_REAL 0
#define CFG_DEFAULT_CPU_THROTTLE_GRANULARITY 10
#define CFG_DEFAULT_CPU_SPEED_FACTOR 1.0f
#define CFG_DEFAULT_CPU_CLOCK_PERIOD 10
#define CFG_DEFAULT_CPU_CLOCK_START 0

#define CFG_DEFAULT_MEMORY_ELWRO_MODULES 4
#define CFG_DEFAULT_MEMORY_MEGA_MODULES 0
#define CFG_DEFAULT_MEMORY_HARDWIRED_SEGMENTS 2
#define CFG_DEFAULT_MEMORY_MEGA_PROM NULL
#define CFG_DEFAULT_MEMORY_MEGA_BOOT 0
#define CFG_DEFAULT_MEMORY_PRELOAD NULL

#define CFG_DEFAULT_FPGA_DEVICE "/dev/ttyUSB0"
#define CFG_DEFAULT_FPGA_SPEED 1000000

#define CFG_DEFAULT_UI_INTERFACE "curses"

#define CFG_DEFAULT_SOUND_ENABLED 0
#define CFG_DEFAULT_SOUND_DRIVER "pulseaudio"
#define CFG_DEFAULT_SOUND_OUTPUT "default"
#define CFG_DEFAULT_SOUND_OUTPUT_FILE "sound.raw"
#define CFG_DEFAULT_SOUND_VOLUME 30
#define CFG_DEFAULT_SOUND_RATE 44100
#define CFG_DEFAULT_SOUND_LATENCY 20
#define CFG_DEFAULT_SOUND_BUFFER_LEN 128
#define CFG_DEFAULT_SOUND_FILTER 1

#define cfg_getdouble iniparser_getdouble
#define cfg_getint iniparser_getint
#define cfg_getstr iniparser_getstring
#define cfg_getbool iniparser_getboolean
#define cfg_contains iniparser_find_entry
#define cfg_set iniparser_set
#define cfg_load iniparser_load
#define cfg_free iniparser_freedict

#endif
