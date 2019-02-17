; OPTS -c configs/iotester.cfg

; Test if I/O channel properly sends all possible IRQs
; Test if sending interrupt specification works

	.cpu	mera400

	.include cpu.inc

	uj	start

	.org	INTV
	.res	32, iotester_iv

	.org	STACKP
	.word	stack

	.org	OS_START

	.include iotester.inc

mask_0:	.word	IMASK_NONE
mask_ch:.word	IMASK_ALL_CH

; ------------------------------------------------
; r3 - current channel number
; r1 - random interrupt specification
start:
	im	mask_ch		; enable all channel interrupts
	lwt	r3, 0		; start from channel 0

loop:	
	lw	r1, r3
	lj	iotester_setchan
	lj	iotester_rnd	; generate random number in r1
	lj	iotester_irq	; request the interrupt request

	cw	r1, r7		; intspec matches?
	bb	r0, ?E
	hlt	040
	awt	r3, 1		; test next channel
	cw	r3, 16		; last channel?
	jn	loop

	im	mask_0
	hlt	077
stack:

; XPCT ir : 0xec3f
