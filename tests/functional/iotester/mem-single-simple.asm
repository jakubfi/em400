; OPTS -c configs/iotester.ini

; Test memory operations: read/write single word

	.cpu	mera400

	.include cpu.inc

	.const	BUF_SIZE 1024
	.const	STACK_SIZE 32*4
	.const	rdbuf prog_end + STACK_SIZE
	.const	wrbuf rdbuf + BUF_SIZE

	uj	start

	.org	INTV
	.res	32, iotester_iv

	.org	OS_START

	.include iotester.inc

mask_0:	.word	IMASK_NONE
mask_ch:.word	IMASK_ALL_CH

; ------------------------------------------------
start:
	lw	r1, prog_end
	rw	r1, STACKP
	im	mask_ch		; enable all channel interrupts

	; fill rdbuf with test data
	lw	r1, rdbuf-1
	lw	r2, BUF_SIZE
floop:	rw	r2, r1+r2
	drb	r2, floop

	; read/write memory
	lw	r1, 14
	lj	iotester_setchan; set iotester channel
	lw	r1, rdbuf
	lj	iotester_wam	; set memory address register to rdbuf
	lw	r1, 0
	lj	iotester_wab	; set buffer addres register to 0
	lw	r1, BUF_SIZE
	lj	iotester_rm	; load data from memory to I/O device
	lw	r1, wrbuf
	lj	iotester_wam	; set memory address register to wrbuf
	lw	r1, BUF_SIZE
	lj	iotester_wm	; write memory

	; compare memory contents
	lw	r1, rdbuf-1
	lw	r2, wrbuf-1
	lw	r4, BUF_SIZE
cmploop:
	lw	r3, [r1+r4]
	cw	r3, [r2+r4]
	jn	fail
	drb	r4, cmploop

	hlt	077
fail:	hlt	050

prog_end:

; XPCT ir : 0xec3f
