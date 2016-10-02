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

// new GCC atomics (C++11-like)
#if defined(__GNUC__) && (defined __ATOMIC_ACQUIRE) && (defined __ATOMIC_RELEASE) && (defined __ATOMIC_SEQ_CST)

	#define atom_load(ptr)					__atomic_load_n(ptr, __ATOMIC_RELAXED)
	#define atom_store(ptr, val)			__atomic_store_n(ptr, val, __ATOMIC_RELAXED)
	#define atom_fence()					__atomic_thread_fence(__ATOMIC_SEQ_CST)

	#define atom_load_acquire(ptr)			__atomic_load_n(ptr, __ATOMIC_ACQUIRE)
	#define atom_store_release(ptr, val)	__atomic_store_n(ptr, val, __ATOMIC_RELEASE)
	#define atom_full_fence()				__atomic_thread_fence(__ATOMIC_SEQ_CST)

// "atomics" on x86
#elif (defined (__x86_64__) || defined (__i386__))

	#define atom_load(ptr)					*(ptr); asm volatile("" ::: "memory")
	#define atom_store(ptr, val)			*(ptr) = (val); asm volatile("" ::: "memory")
	#define atom_fence()					asm volatile("mfence" ::: "memory")

	#define atom_load_acquire(ptr)			*(ptr); asm volatile("" ::: "memory")
	#define atom_store_release(ptr, val)	*(ptr) = (val); asm volatile("" ::: "memory")
	#define atom_full_fence()				asm volatile("mfence" ::: "memory")

// other architectures and compilers
#else

	#error "Don't know how to handle atomic loads/stores with your compiler (and/or on your architecture)"

#endif

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
