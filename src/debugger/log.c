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

#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <pthread.h>

#include "cfg.h"
#include "errors.h"
#include "utils.h"
#include "debugger/log.h"
#include "cpu/cpu.h"
#include "cpu/registers.h"

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

char *log_dname[] = {
	"REG",
	"MEM",
	"CPU",
	"OP",
	"INT",
	"IO",
	"MX",
	"PX",
	"CCHR",
	"CMEM",
	"TERM",
	"WNCH",
	"FLOP",
	"PNCH",
	"PNRD",
	"CRK5",
	NULL
};

char *log_reg_name[] = {
	"R0",
	"R1",
	"R2",
	"R3",
	"R4",
	"R5",
	"R6",
	"R7",
	"IC",
	"SR",
	"IR",
	"KB",
	"MOD",
	"MODc",
	"ZC17",
	"ALARM"
};

char *log_int_name[] = {
	"CPU power loss",
	"memory parity error",
	"no memory",
	"2nd CPU high",
	"ext power loss",
	"timer/special",
	"illegal instruction",
	"AWP div overflow",
	"AWP underflow",
	"AWP overflow",
	"AWP div/0",
	"special/timer",
	"chan 0",
	"chan 1",
	"chan 2",
	"chan 3",
	"chan 4",
	"chan 5",
	"chan 6",
	"chan 7",
	"chan 8",
	"chan 9",
	"chan 10",
	"chan 11",
	"chan 12",
	"chan 13",
	"chan 14",
	"chan 15",
	"OPRQ",
	"2nd CPU low",
	"software high",
	"software low"
};

char *log_io_result[] = {
	"NO DEVICE",
	"ENGAGED",
	"OK",
	"PARITY ERROR"
};

char *log_fname;

FILE *log_f;

int log_enabled = 0;
int log_level[L_MAX] = { 0 };

const char *log_int_indent = "--> --> --> --> --> --> --> --> ";
int log_int_level = LOG_INT_INDENT_MAX;

// -----------------------------------------------------------------------
int log_open(const char *logf)
{
	log_close();
	log_f = fopen(logf, "a");
	if (!log_f) {
		return E_LOG_OPEN;
	}
	log_fname = strdup(logf);
	return E_OK;
}

// -----------------------------------------------------------------------
void log_close()
{
	if (log_f) {
		fclose(log_f);
		log_f = NULL;
		free(log_fname);
		log_fname = NULL;
	}
}

// -----------------------------------------------------------------------
int log_enable()
{
	if (!log_f) {
		return E_LOG_OPEN;
	}

	log_enabled = 1;
	return E_OK;
}

// -----------------------------------------------------------------------
void log_disable()
{
	log_enabled = 0;
}

// -----------------------------------------------------------------------
int log_find_domain(char *name)
{
	if (!strcasecmp(name, "all")) {
		return -1;
	}

	char **d = log_dname;
	int i = 0;
	while (*(d+i)) {
		if (!strcasecmp(name, *(d+i))) {
			return i;
		}
		i++;
	}
	return -666;
}

// -----------------------------------------------------------------------
void log_setlevel(int domain, int level)
{
	if (domain == -1) {
		for (int i=0 ; i<L_MAX ; i++) {
			log_level[i] = level;
		}
	} else {
		log_level[domain] = level;
	}
}

// -----------------------------------------------------------------------
void log_pretty_log(int domain, int level, char *pre, char *text)
{
	char now[30+1];
	struct timeval ct;

	if (log_int_level > LOG_INT_INDENT_MAX) {
		log_int_level = LOG_INT_INDENT_MAX;
	}

	gettimeofday(&ct, NULL);
	strftime(now, 30, "%Y-%m-%d %H:%M:%S", localtime(&ct.tv_sec));

	if ((domain == L_CPU) || (domain == L_MEM) || (domain == L_INT) || (domain == L_OP) || (domain == L_IO) || (domain == L_CRK5)) {
		uint16_t bprog;
		uint16_t pname[2];
		mem_get(0, 0x62, &bprog);
		mem_mget(0, bprog+52, pname, 2);
		char *n1 = int2r40(pname[0]);
		char *n2 = int2r40(pname[1]);
		fprintf(log_f, "%s | %3i %-4s | %i:%-2i 0x%04x | %s%s | ", now, level, log_dname[domain], Q, NB, cycle_ic, n1, n2);
		fprintf(log_f, "%s%s%s\n", log_int_indent+log_int_level, pre, text);
	} else {
		fprintf(log_f, "%s | %3i %-4s |             |        | ", now, level, log_dname[domain]);
		fprintf(log_f, "%s%s\n", pre, text);
	}
}

// -----------------------------------------------------------------------
void log_log(int domain, int level, char *format, ...)
{
	if (!log_enabled || !log_f || (log_level[domain] < level)) {
		return;
	}

	char buf[1024];
	va_list vl;
	va_start(vl, format);
	vsprintf(buf, format, vl);
	va_end(vl);

	pthread_mutex_lock(&log_mutex);
	log_pretty_log(domain, level, "", buf);
	pthread_mutex_unlock(&log_mutex);
	fflush(log_f);
}

// -----------------------------------------------------------------------
void log_splitlog(int domain, int level, char *text)
{
	char *p;
	char *start = text;

	if (!log_enabled || !log_f || (log_level[domain] < level)) {
		return;
	}

	pthread_mutex_lock(&log_mutex);
	log_pretty_log(domain, level, "", " ,---------------------------------------------------------");
	while (start && *start) {
		p = strchr(start, '\n');
		if (p) {
			*p = '\0';
			log_pretty_log(domain, level, " | ", start);
			start = p+1;
		} else {
			log_pretty_log(domain, level, " | ", start);
			start = NULL;
		}
	}
	log_pretty_log(domain, level, "", " `---------------------------------------------------------");
	pthread_mutex_unlock(&log_mutex);
	fflush(log_f);
}

// vim: tabstop=4 shiftwidth=4 autoindent
