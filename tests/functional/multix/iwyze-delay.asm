; OPTS -c configs/multix.cfg

; IWYZE MULTIX interrupt should be delayed

	.equ	stackp 0x61
	.equ	prog_beg 0x70
	.equ	int_mx 0x40 + 12 + 1
	.equ	int_tim 0x40 + 5
	.equ	int_ex 0x40 + 11
        .equ    unmask_chan  0b0000010000000000
        .equ    unmask_timer 0b0000110000000000

	.equ	iwyze 0b0000001000000000
	.equ	mx_chan 1

	UJ	start

stack:	.res	8
maskc:	.word	unmask_chan
maskt:	.word	unmask_timer
tcount:	.word	-50		; we'll wait 0.5s for multix interrupt (50 x timer tick)

	.org	prog_beg

; ------------------------------------------------------------------------
tim_proc:
	IB	tcount		; allow only <-tcount> timer interrupts
	LIP
	HLT	044		; then die

; ------------------------------------------------------------------------
mx_proc:
	LW	r4, [stackp]
	LW	r4, [r4-1]
	CW	r4, iwyze	; is it IWYZE?
	BB	r0, ?E
	HLT	042
	CW	r7, [tcount]	; is it after a timer interrupt?
	BLC	?E
	HLT	043
	HLT	077		; we're good

; ------------------------------------------------------------------------
start:
	LW	r3, stack
	RW	r3, stackp
	LW	r3, mx_proc
	RW	r3, int_mx
	LW	r3, tim_proc
	RW	r3, int_tim
	RW	r3, int_ex

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
