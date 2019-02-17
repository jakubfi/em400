; OPTS -c configs/mod.cfg

; Vanilla CPU masks all interrupts below and including current one.
; Modified (MX-16) CPU handles interrupt masking differently:
; any channel interrupt causes masking interrupts 5-31.

	.cpu	mx16

	.include cpu.inc

	.equ	int_mx INTV_CH1

	uj	start

mask:	.word	IMASK_PARITY | IMASK_NOMEM | IMASK_CPU_H | IMASK_IFPOWER | IMASK_GROUP_H | IMASK_CH0_1
zmask:	.word	0
raise:	.word	1\3

	.org	INTV
	.res	32, empty

	.org	OS_START
start:
	rz	0x200
	rz	0x201
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, cpu2x
	rw	r1, INTV_CPU_H
	lw	r1, mx
	rw	r1, int_mx
	lwt	r6, 0

; first, check int masking for vanilla CPU (store mask at [0x200])

	lw	r7, 0x200
	im	mask
w1:	hlt
	bb	r6, ?X	; did we wake up because of MULTIX interrupt?
	ujs	w1

	er	r6, ?X
	im	zmask
	mcl

; then, repeat for MX-16 CPU (store mask at [0x201])

	cron
	awt	r7, 1
	im	mask
w2:	hlt
	bb	r6, ?X	; did we wake up because of MULTIX interrupt?
	ujs	w2
	hlt	077

; we use simulated 2nd CPU interrupt to get interrupt mask
; that was set by previous interrupt
cpu2x:	md	[STACKP]
	lw	r1, [-2]
	rw	r1, r7	; store previous int mask
	lip

; we use multix as a source of channel interrupt
; (as it always sends interrupt after initialization)
mx:	fi	raise	; raise 2nd CPU interrupt
	or	r6, ?X	; indicate that we've received MULTIX interrupt
; all other interrupts do nothing
empty:	lip
stack:

; XPCT [0x200] : 0b1111100000000000
; XPCT [0x201] : 0b1111000000000000
