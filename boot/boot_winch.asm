; Bootstrap MERA-400 CROOK-5 os from winchester disk

; Copyright (c) 2013 Jakub Filipowicz <jakubf@gmail.com>
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc.,
; 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

; ------------------------------------------------------------------------
	.equ chan 1
	.equ chan_mask 0b0000010000000000
	.equ mx 1
	.equ winch 0
	.equ mx_int 0x40 + 12 + mx
	.equ stackp 0x61
	.equ prog_beg 0x70

	uj start

; ------------------------------------------------------------------------
stack:	.res 2*4
mask:	.word chan_mask

mx_cmd:	.word 0b101\2+chan\14, 0b010\2+chan\14, 0b100\2+chan\14
mx_ps:	.word psuk, 0, pstrx
intseq:	.word 2, 5, 10, 13

psuk:	.word 2\7 + 1
	.word 0
	.word 27
	.word 1\3 + 3\7
	.word 6\7 + 28
	.word 1\7 + 1
pstrx:	.word 2\7
	.word image
	.word imagesize-1
	.word 0
	.word 1

	.res 0x40-.	; padding up to 0x40

; ------------------------------------------------------------------------
	.org prog_beg
pmx:
	irb r7, 0	; int_sequence++

	md [stackp]
	lw r1, [-1]
	lw r2, r1
	zlb r1		; r1 = device
	zrb r2
	shc r2, 8	; r2 = intspec

	cw r2, [intseq+r7]
	bb r0, ?E	; proper int sequence?
	hlt 2

	cw r2, 2	; is this the interrupt after multix reset (=2)?
	jes cont	; don't check device number

	cw r1, winch
	bb r0, ?E	; is this the right device?
	hlt 3
cont:
	cw r7, 3	; done loading?
	je boot
	lw r1, [mx_ps+r7]
	ou r1, [mx_cmd+r7]
	.word 0, 0, next, 0
next:
	lip

; ------------------------------------------------------------------------
boot:
	la lcp
	ra copier	; copy 7 words from @lcp to @copier
	lw r1, -imagesize
	lw r3, image
	uj copier	; run copier

; ------------------------------------------------------------------------
lcp:	
	lw r2, [image+imagesize+r1]
	rw r2, imagesize+r1
	irb r1, lcp
	uj 0		; start system
elcp:

; ------------------------------------------------------------------------
; ---- MAIN --------------------------------------------------------------
; ------------------------------------------------------------------------
start:
	mcl
	lw r7, -1
	lwt r1, stack
	rw r1, stackp
	lw r1, pmx
	rw r1, mx_int

	im mask
lhlt:
	hlt 1
	ujs lhlt	; forever waiting for interrupts

; ------------------------------------------------------------------------
image:
	.equ memsize 2*4096
	.equ imagesize (memsize-image)-(elcp-lcp)
	.equ copier image+imagesize

