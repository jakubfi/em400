; OPTS -c configs/multix.cfg
; PRECMD CLOCK ON

; IWYZE MULTIX interrupt should be delayed

	.include cpu.inc
	.include multix.inc

	uj	start

stack:	.res	8
mask0:	.word	0
maskc:	.word	IMASK_CH0_1
maskt:	.word	IMASK_GROUP_H | IMASK_CH0_1
tcount:	.word	-50		; we'll wait 0.5s for multix interrupt (50 x timer tick)

	.org	OS_START

; ------------------------------------------------------------------------
tim_proc:
	ib	tcount		; allow only <-tcount> timer interrupts
	lip
	hlt	044		; then die

; ------------------------------------------------------------------------
mx_proc:
	im	mask0
	lw	r4, [STACKP]
	lw	r4, [r4-1]
	cw	r4, MX_IWYZE	; is it IWYZE?
	bb	r0, ?E
	hlt	042
	cw	r7, [tcount]	; is it after a timer interrupt?
	blc	?E
	hlt	043
	hlt	077		; we're good

; ------------------------------------------------------------------------
start:
	lw	r3, stack
	rw	r3, STACKP
	lw	r3, mx_proc
	rw	r3, INTV_CH1
	lw	r3, tim_proc
	rw	r3, INTV_TIMER
	rw	r3, INTV_UNUSED

	lw	r7, [tcount]

	; we need to take care of the fact, that the timer interrupt has higher priority
	; than channel interrupts and is always served before MX int, even if
	; MX int comes first
	im	maskc	; first, install mask to enable channel interrupts only
			; if MX interrupt is already there, it's bad
	IM	maskt	; then, enable timer interrupt too and wait for MX int

loop:	hlt		; wait for multix interrupt (or timeout)
	ujs	loop

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT ir : 0xec3f
