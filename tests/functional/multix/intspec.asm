; OPTS -c configs/multix.cfg
; PRECMD CLOCK ON

; INTSPEC should:
;  * always return OK
;  * give 0 before initialization
;  * give IWYZE after initialization
;  * give IWYZE when executed 'by hand' after initialization with interrupts disabled
;  * give 0 afterwards

	.include cpu.inc
	.include io.inc
	.include multix.inc

	uj	start

zmask:	.word	IMASK_NONE
cmask:	.word	IMASK_CH0_1
tmask:	.word	IMASK_GROUP_H

	.org	OS_START

; ------------------------------------------------------------------------
mx_proc:
	lw	r4, [STACKP]
	lw	r4, [r4-1]
	cw	r4, MX_IWYZE	; is it IWYZE?
	bb	r0, ?E
	hlt	050		; not iwyze
	lip

; ------------------------------------------------------------------------
tim_proc:
	awt	r7, 1
	lip

; ------------------------------------------------------------------------
get_intspec:
	lwt	r5, -13
	in	r5, MX_CMD_INTSPEC | 1\IO_CHAN
	.word	f, f, ok, f
f:	uj	r3
ok:	shc	r5, 8
	cw	r5, r2
	bb	r0, ?E
	uj	r3
	uj	r3+1
	
; ------------------------------------------------------------------------
start:
	lw	r3, stack
	rw	r3, STACKP
	lw	r3, mx_proc
	rw	r3, INTV_CH1
	lw	r3, tim_proc
	rw	r3, INTV_TIMER

	lwt	r7, 0	; reset wait timer counter

	; "intspec" before initialization = 0
	lw	r2, 0
	rj	r3, get_intspec
	hlt	041

	im	cmask
	hlt		; wait for mx int

	; "intspec" after acknowledged initialization = 0
	lw	r2, 0
	rj	r3, get_intspec
	hlt	042

	; reset multix
	in	r2, MX_CMD_RESET | 1\IO_CHAN
	.word	f3, f3, ok3, f3
f3:	hlt	043
ok3:

	; wait 200ms
	im	tmask	; disable channel interrupts, enable timer
loop:	hlt
	cw	r7, 30	; wait 30x10ms for multix to initialize
	bb	r0, ?E
	ujs	loop

	; "intspec" after not acknowledged initialization = IWYZE
	lw	r2, MX_IWYZE
	shc	r2, 8
	rj	r3, get_intspec
	hlt	044

	; "intspec" after "manually" acknowledged initialization = 0
	lw	r2, 0
	rj	r3, get_intspec
	hlt	045

	im	zmask
	hlt	077

stack:

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT ir : 0xec3f
