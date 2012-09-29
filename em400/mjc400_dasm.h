//  Copyright (c) 2012 Jakub Filipowicz <jakubf@gmail.com>
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

int mjc400_dasm(uint16_t* memptr, char **buf);

int mjc400_dasm_illegal(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_dasm_2argn(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_dasm_37(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_dasm_ka1(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_dasm_70(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_dasm_71(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_dasm_72(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_dasm_73(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_dasm_74(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_dasm_75(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_dasm_76(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_dasm_77(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_dasm_fd(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_dasm_js(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_dasm_ka2(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_dasm_c(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_dasm_c_shc(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_dasm_s(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_dasm_j(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_dasm_l(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_dasm_g(uint8_t op, uint16_t* memptr, char **buf);
int mjc400_dasm_bn(uint8_t op, uint16_t* memptr, char **buf);

