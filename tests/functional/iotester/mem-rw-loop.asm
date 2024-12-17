; OPTS -c configs/iotester-mega.ini
; PRECMD CLOCK ON

	.cpu	mera400

	.include cpu.inc
	.include io.inc
	.include mega.inc

	.const	BUF_SIZE 1024
	.const	STACK_SIZE 32*4
	.const	rdbuf prog_end + STACK_SIZE
	.const	wrbuf rdbuf + BUF_SIZE

	uj	start

offset:	.res	1
len:	.res	1

	.org	OS_START

	.include iotester.inc
	.include prng.inc

mask_0:	.word	IMASK_NONE
mask_ch:.word	IMASK_ALL_CH

; ------------------------------------------------
; configure two random MEGA frames at pages 0:e000 and 0:f000
reconfigure_mem:
	.res	1

	lj	rand ; [r1, r2] = random

.page14:
	lw	r3, 14\MEM_PAGE | 0\MEM_SEGMENT ; logical address specification
	nr	r2, 15\MEM_FRAME | 15\MEM_MODULE ; leave random physical address in r2
	or	r2, MEM_CFG | MEGA_ALLOC_DONE | MEGA_EPROM_HIDE | MEGA_ALLOC ; "or" memory command
	ou	r3, r2 ; send to MEGA
	.word	.no, .en, .page15, .pe
.no:
.en:
.pe:	hlt	060
	ujs	.pe

.page15:
	lw	r3, 15\MEM_PAGE | 0\MEM_SEGMENT
	nr	r1, 15\MEM_FRAME | 15\MEM_MODULE
	or	r1, MEM_CFG | MEGA_ALLOC_DONE | MEGA_EPROM_HIDE | MEGA_ALLOC
	ou	r3, r1
	.word	.no, .en, .fin, .pe
.fin:
	uj	[reconfigure_mem]

; ------------------------------------------------
; select random offset and len
randomize:
	.res	1

	lj	rand

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

	lj	rand ; r1 = initial random

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
	lj	iotester_wam	; set memory address register to rdbuf
	lw	r1, 0
	lj	iotester_wab	; set buffer addres register to 0
	lw	r1, [len]
	lj	iotester_rm	; load data from memory to I/O device

	uj	[copy_to]

; ------------------------------------------------
; copy memory from iotester
copy_from:
	.res	1

	lw	r1, 0xf000
	aw	r1, [offset]
	lj	iotester_wam    ; set memory address register to wrbuf
	lw	r1, [len]
	lj	iotester_wm     ; write memory

	uj	[copy_from]

; ------------------------------------------------
; compare original memory to copied throug iotester
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
	cw	r5, [r2+r3]
	jn	r6
	drb	r3, .loop_cmp

	uj	[compare]

.failcmp:
	hlt	051
	ujs	.failcmp

; ------------------------------------------------
start:
	; seed prng
	lj	prngseed
	im	mask_0

	; setup interrupts
	lw	r1, iotester_iv
	rw	r1, INTV_CH14
	lw	r1, stack
	rw	r1, STACKP
	im	mask_ch		; enable all channel interrupts

	; setup iotester
	lw	r1, 14
	lj	iotester_setchan; set iotester channel

	lw	r1, -10000 ; 10000 loops
	rw	r1, loop_counter
test:
	lj	reconfigure_mem
	lj	randomize
	lj	rnd_fill
	lj	copy_to
	lj	copy_from
	lj	compare
	; comment out instruction below to get infinite loop
	ib	loop_counter
	ujs	test

.done:	hlt	077
	ujs	.done
.fail:	hlt	050
	ujs	.fail

loop_counter:
	.res	1

stack:	.res	32*4

prog_end:

; XPCT ir : 0xec3f
