//  Copyright (c) 2014 Jakub Filipowicz <jakubf@gmail.com>
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

#ifndef ATOMIC_H
#define ATOMIC_H

#if (defined __ATOMIC_SEQ_CST) && (defined __ATOMIC_RELAXED)
	#define atom_load(ptr) __atomic_load_n(ptr, __ATOMIC_RELAXED)
	#define atom_store(ptr, val) __atomic_store_n(ptr, val, __ATOMIC_RELAXED)
	#define atom_fence() __atomic_thread_fence(__ATOMIC_SEQ_CST)
#elif (defined __GCC_HAVE_SYNC_COMPARE_AND_SWAP_1) || (defined __GCC_HAVE_SYNC_COMPARE_AND_SWAP_2) || (defined __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4) || (defined __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8)
	#define atom_load(ptr) __sync_fetch_and_or(ptr, 0)
	#define atom_store(ptr, val) __sync_val_compare_and_swap(ptr, *ptr, val)
	#define atom_fence() __sync_synchronize()
#else
	#error "Don't know how to handle atomic loads/stores with your compiler (and/or on your architecture)"
#endif

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
