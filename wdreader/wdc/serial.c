//  Copyright (c) 2013 Jakub Filipowicz <jakubf@gmail.com>
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

#include <avr/io.h>
#include <util/delay.h>

#include "serial.h"

#define BAUD    9600
#define UBRR	(F_CPU / (16UL*BAUD) - 1)

// -----------------------------------------------------------------------
void serial_init(void)
{
	UBRR0H = (unsigned char) UBRR >> 8;
	UBRR0L = (unsigned char) UBRR;
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);
	UCSR0C = (1 << URSEL0) | (1 << UCSZ01) | (1 << UCSZ00);
}

// -----------------------------------------------------------------------
void serial_tx_char(char c)
{
	while (!(UCSR0A & (1 <<UDRE0)));
	UDR0 = c;
}

// -----------------------------------------------------------------------
void serial_tx_string(char *data)
{
	while ((*data != '\0')) {
		serial_tx_char(*data);
		data++;
	}   
}

// -----------------------------------------------------------------------
char serial_rx_char(void)
{
	while (!(UCSR0A & (1<<RXC0)));
	return UDR0;
}

// vim: tabstop=4
