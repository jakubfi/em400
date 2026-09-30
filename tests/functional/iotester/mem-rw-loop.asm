; CONFIG configs/iotester-mega.ini
; PRECMD CLOCK ON

; CPU - I/O stress test. Transfers data through iotester and compares.
; Short run by default, can be used for longer soaks to look
; for threading issues.

	.cpu	mera400

	.include cpu.inc
	.include io.inc
	.include mega.inc

	.const	DURATION 200 ; timer ticks
	.const	MCFG MEM_CFG | MEGA_ALLOC_DONE | MEGA_EPROM_HIDE | MEGA_ALLOC

	uj	start

offset:	.res	1
len:	.res	1

	.org	OS_START

	.include iotester.inc
	.include prng.inc

; timer must stay masked around iotester commands: waking up the HLT
; in __it_cmd__ would return before the transfer ends
mask_0:	.word	IMASK_NONE
mask_io:.word	IMASK_ALL_CH
mask_cpu:
	.word	IMASK_ALL_CH | IMASK_GROUP_H
seedv:	.dword	0x1d872b41
ticks:	.word	-DURATION

tick:	ib	ticks
	nop
	lip

illegal:
	hlt	052

; ------------------------------------------------
; map two different random MEGA frames at pages 0:e000 and 0:f000
reconfigure_mem:
	.res	1

	lj	urand ; [r1, r2] = random

	; module 0 is Elwro in this config, keep MEGA frames to modules 1-8
	nr	r1, 15\MEM_FRAME | 7\MEM_MODULE
	aw	r1, 1\MEM_MODULE
	nr	r2, 15\MEM_FRAME | 7\MEM_MODULE
	aw	r2, 1\MEM_MODULE
	cw	r1, r2
	jn	.differ
	xr	r1, 1\MEM_FRAME
.differ:
	or	r1, MCFG
	or	r2, MCFG

	lw	r3, 14\MEM_PAGE | 0\MEM_SEGMENT
	ou	r3, r2
	.word	.no, .en, .page15, .pe
.no:
.en:
.pe:	im	mask_0
	hlt	060

.page15:
	lw	r3, 15\MEM_PAGE | 0\MEM_SEGMENT
	ou	r3, r1
	.word	.no, .en, .fin, .pe
.fin:
	uj	[reconfigure_mem]

; ------------------------------------------------
; select random offset and len
randomize:
	.res	1

	lj	urand

	nr	r1, 0x0fff ; r1 = random offset
	rw	r1, offset
	nr	r2, 0x0fff

	cwt	r2, 0
	jgs	.len_min_ok
	awt	r2, 1
	ujs	.len_max_ok

.len_min_ok:
	lw	r3, 0x1000
	sw	r3, r1 ; r3 = max length
	cw	r2, r3 ; > max?
	jls	.len_max_ok
	lw	r2, r3 ; truncate to max
.len_max_ok:
	rw	r2, len
	uj	[randomize]

; ------------------------------------------------
; fill memory with "random" data
rnd_fill:
	.res	1

	lj	urand ; r1 = initial random

	lw	r5, 0xe000 - 1
	aw	r5, [offset]
	lw	r4, [len]
.loop:	rw	r1, r5 + r4
	awt	r1, 1
	drb	r4, .loop

	uj	[rnd_fill]

; ------------------------------------------------
; copy memory to iotester
copy_to:
	.res	1

	lw	r1, 0xe000
	aw	r1, [offset]
	lj	iotester_wam	; set memory address register
	lw	r1, 0
	lj	iotester_wab	; set buffer addres register to 0
	lw	r1, [len]
	lj	iotester_rm	; load data from memory to I/O device

	uj	[copy_to]

; ------------------------------------------------
; invert source memory right after copy_to returns, to catch the iotester
; reporting the read done before it finished reading
invert_src:
	.res	1

	lw	r5, 0xe000 - 1
	aw	r5, [offset]
	lw	r4, [len]
.loop:	lw	r1, [r5 + r4]
	xr	r1, -1
	rw	r1, r5 + r4
	drb	r4, .loop

	uj	[invert_src]

; ------------------------------------------------
; copy memory from iotester
copy_from:
	.res	1

	lw	r1, 0xf000
	aw	r1, [offset]
	lj	iotester_wam    ; set memory address register
	lw	r1, [len]
	lj	iotester_wm     ; write memory

	uj	[copy_from]

; ------------------------------------------------
; compare inverted source memory to copied throug iotester
compare:
	.res	1

	lw	r1, [offset] ; source address
	aw	r1, 0xe000-1
	lw	r2, [offset] ; copy address
	aw	r2, 0xf000-1
	lw	r3, [len]
	lw	r6, .failcmp

.loop_cmp:
	lw	r5, [r1+r3]
	xr	r5, -1
	cw	r5, [r2+r3]
	jn	r6
	drb	r3, .loop_cmp

	uj	[compare]

.failcmp:
	im	mask_0
	hlt	051

; ------------------------------------------------
start:
	ld	seedv
	lj	seed

	lw	r1, iotester_iv
	rw	r1, INTV_CH14
	lw	r1, tick
	rw	r1, INTV_TIMER
	lw	r1, illegal
	rw	r1, INTV_ILLEGAL
	lw	r1, stack
	rw	r1, STACKP

	lw	r1, 14
	lj	iotester_setchan

test:
	im	mask_cpu
	lj	reconfigure_mem
	lj	randomize
	lj	rnd_fill
	im	mask_io
	lj	copy_to
	im	mask_cpu
	lj	invert_src
	im	mask_io
	lj	copy_from
	im	mask_cpu
	lj	compare
	lw	r1, [ticks]
	cwt	r1, 0
	jl	test

	im	mask_0
	hlt	077

stack:	.res	32*4
