//  Copyright (c) 2016 Jakub Filipowicz <jakubf@gmail.com>
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
#include <inttypes.h>

#include "ectl_emu.h"

#include "log.h"

// -----------------------------------------------------------------------
int ectl_callback_register(int callback, ectl_callback_f callback_f)
{
	return 666;
}

// -----------------------------------------------------------------------
uint16_t ectl_capabilities_get()
{
	return 666;
}

// -----------------------------------------------------------------------
void ectl_log_state_set(int state)
{
	if (state == ECTL_ON) {
		log_enable();
	} else {
		log_disable();
	}
}

// -----------------------------------------------------------------------
int ectl_log_state_get()
{
	if (log_is_enabled()) {
		return ECTL_ON;
	} else {
		return ECTL_OFF;
	}
}

// -----------------------------------------------------------------------
int ectl_log_level_set(int component, int level)
{
	return 666;
}

// -----------------------------------------------------------------------
int ectl_log_level_get(int component)
{
	return log_get_level(component);
}

// -----------------------------------------------------------------------
int ectl_cpu_count()
{
	return 1;
}
// -----------------------------------------------------------------------
int ectl_mem_frames_get(int type, int module)
{
	return 666;
}

// -----------------------------------------------------------------------
uint16_t ectl_mem_layout_get(int nb)
{
	return 666;
}

// -----------------------------------------------------------------------
int ectl_mem_mega_prom_state_get()
{
	return 666;
}

// -----------------------------------------------------------------------
uint32_t ectl_interrupts_get()
{
	return 666;
}

// -----------------------------------------------------------------------
void ectl_interrupt_set(int interrupt)
{

}

// -----------------------------------------------------------------------
void ectl_interrupt_clear(int interrupt)
{

}

// vim: tabstop=4 shiftwidth=4 autoindent
