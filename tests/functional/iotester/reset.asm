; OPTS -c configs/iotester.ini

; Test if I/O channel receives "clear" on the interface

	.cpu	mera400

	.include cpu.inc

	uj	start

	.org	INTV
	.res	32, io_iv

	.org	OS_START

	.include iotester.inc

mask_0:	.word	IMASK_NONE
mask_ch:.word	IMASK_ALL_CH

io_iv:	md	[STACKP]
	lw	r1, [-1]
	cw	r1, 0xffff	; is this the reset interrupt?
	jn	wrongspec
	awt	r7, 1
	lip
wrongspec:
	im	mask_0
	hlt	040
	ujs	wrongspec

; ------------------------------------------------
; r7: current interrupt count
start:
	lwt	r7, 0
	lw	r1, stack
	rw	r1, STACKP

; enable reset interrupt on all iotester channels

	lwt	r1, 0
chloop:
	lj	iotester_setchan
	lj	iotester_eri
	awt	r1, 1
	cwt	r1, 16
	jl	chloop

	mcl
	im	mask_ch		; enable all channel interrupts

; wait for 16 interrupts from all iotester channels

forever:
	hlt
	cw	r7, 16
	jl	forever
	im	mask_0
	hlt	077

stack:

; XPCT ir : 0xec3f
; XPCT r7 : 16
