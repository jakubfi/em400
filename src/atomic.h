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

// check compiler/architecture

#if defined(__GNUC__)
	#if defined(__ATOMIC_ACQUIRE) && defined(__ATOMIC_RELEASE) && defined(__ATOMIC_SEQ_CST)
		#define ATOMIC_H_GCC_NEW_ANY
	#elif defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4)
		#if defined(__x86_64__) || defined(__i386__)
			#define ATOMIC_H_GCC_OLD_X86
		#else
			#define ATOMIC_H_GCC_OLD_ANY
		#endif
	#endif
#endif

// setup macros

// new GCC atomics (C++11-like)
#if defined(ATOMIC_H_GCC_NEW_ANY)
	#define atom_load_acquire(ptr)			__atomic_load_n(ptr, __ATOMIC_ACQUIRE)
	#define atom_store_release(ptr, val)	__atomic_store_n(ptr, val, __ATOMIC_RELEASE)
	#define atom_or_release(ptr, val)		__atomic_or_fetch(ptr, val, __ATOMIC_RELEASE)
	#define atom_and_release(ptr, val)		__atomic_and_fetch(ptr, val, __ATOMIC_RELEASE)
	#define atom_full_fence()				__atomic_thread_fence(__ATOMIC_SEQ_CST)
// old GCC atomics on x86
#elif defined(ATOMIC_H_GCC_OLD_X86)
	#define atom_load_acquire(ptr)			*(ptr); asm volatile("" ::: "memory")
	#define atom_store_release(ptr, val)	*(ptr) = (val); asm volatile("" ::: "memory")
	#define atom_or_release(ptr, val)		__sync_or_and_fetch(ptr, val)
	#define atom_and_release(ptr, val)		__sync_and_and_fetch(ptr, val)
	#define atom_full_fence()				asm volatile("mfence" ::: "memory")
// old GCC atomics
#elif defined(ATOMIC_H_GCC_OLD_ANY)
	#define atom_load_acquire(ptr)			__sync_fetch_and_or(ptr, 0)
	#define atom_store_release(ptr, val)	__sync_val_compare_and_swap(ptr, *ptr, val)
	#define atom_or_release(ptr, val)		__sync_or_and_fetch(ptr, val)
	#define atom_and_release(ptr, val)		__sync_and_and_fetch(ptr, val)
	#define atom_full_fence()				__sync_synchronize()
// other architectures and compilers
#else
	#error "Don't know how to handle atomic operations with your compiler and/or on architecture"
#endif

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
