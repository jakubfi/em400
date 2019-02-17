; OPTS -c configs/multix.cfg
; PRECMD CLOCK ON

; IWYZE MULTIX interrupt should be delayed

	.include cpu.inc
	.include multix.inc

	UJ	start

stack:	.res	8
mask0:	.word	0
maskc:	.word	IMASK_CH0_1
maskt:	.word	IMASK_GROUP_H | IMASK_CH0_1
tcount:	.word	-50		; we'll wait 0.5s for multix interrupt (50 x timer tick)

	.org	OS_START

; ------------------------------------------------------------------------
tim_proc:
	IB	tcount		; allow only <-tcount> timer interrupts
	LIP
	HLT	044		; then die

; ------------------------------------------------------------------------
mx_proc:
	IM	mask0
	LW	r4, [STACKP]
	LW	r4, [r4-1]
	CW	r4, MX_IWYZE	; is it IWYZE?
	BB	r0, ?E
	HLT	042
	CW	r7, [tcount]	; is it after a timer interrupt?
	BLC	?E
	HLT	043
	HLT	077		; we're good

; ------------------------------------------------------------------------
start:
	LW	r3, stack
	RW	r3, STACKP
	LW	r3, mx_proc
	RW	r3, INTV_CH1
	LW	r3, tim_proc
	RW	r3, INTV_TIMER
	RW	r3, INTV_UNUSED

	LW	r7, [tcount]

	; we need to take care of the fact, that the timer interrupt has higher priority
	; than channel interrupts and is always served before MX int, even if
	; MX int comes first
	IM	maskc	; first, install mask to enable channel interrupts only
			; if MX interrupt is already there, it's bad
	IM	maskt	; then, enable timer interrupt too and wait for MX int

loop:	HLT		; wait for multix interrupt (or timeout)
	UJS	loop

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT ir : 0xec3f
