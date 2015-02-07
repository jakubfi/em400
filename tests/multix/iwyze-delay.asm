; OPTS -c configs/multix.cfg

; IWYZE MULTIX interrupt should be delayed 13ms = after first timer interrupt

	.equ	stackp 0x61
	.equ	prog_beg 0x70
	.equ	int_mx 0x40 + 12 + 1
	.equ	int_tim 0x40 + 5
	.equ	int_ex 0x40 + 11
	.equ	unmask_chan 0b0000011110000000
	.equ	unmask_timer 0b0000111110000000
	.equ	iwyze 0b0000001000000000
	.equ	mx_chan 1

	UJ	start

stack:	.res	4
mask:	.word	unmask_chan
maskt:	.word	unmask_timer

	.org	prog_beg

; ------------------------------------------------------------------------
tim_proc:
	LWT	r7, 1
	LIP

; ------------------------------------------------------------------------
mx_proc:
	LW	r4, [stackp]
	LW	r4, [r4-1]
	CW	r4, iwyze	; is it IWYZE?
	JN	bad_int
	CW	r7, 1		; is it after timer interrupt?
	JN	too_early
	HLT	077		; we're good
bad_int:
	HLT	042
too_early:
	HLT	043

; ------------------------------------------------------------------------
start:
	LW	r3, stack
	RW	r3, stackp
	LW	r3, mx_proc
	RW	r3, int_mx
	LW	r3, tim_proc
	RW	r3, int_tim
	RW	r3, int_ex

	; we need to take care of the fact, that timer interrupt has higher priority
	; than channel interrupts and is always served before MX int, even if
	; MX int comes first

	IM	mask	; first, install mask to enable channel interrupts only
			; if MX interrupt is already here, it's bad
	IM	maskt	; then, enable timer interrupt too
loop:	HLT
	UJS	loop

; XPCT int(rz[15]) : 0
; XPCT int(rz[6]) : 0
; XPCT int(alarm) : 0
; XPCT int(r7) : 1
; XPCT oct(ir[10-15]) : 077
