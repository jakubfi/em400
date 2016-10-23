; OPTS -c configs/multix.cfg

; If reset is done before previous reset routine finished, no IWYZE should appear
; (except the final one)

	.equ	stackp 0x61
	.equ	prog_beg 0x70
	.equ	int_mx 0x40 + 12 + 1
	.equ	int_tim 0x40 + 5
	.equ	int_ex 0x40 + 11
	.equ	unmask_chan 0b0000111110000000
	.equ	iwyze 0b0000001000000000
	.equ	mx_chan 1

	UJ	start

stack:	.res	4
mask:	.word	unmask_chan

	.org	prog_beg

; ------------------------------------------------------------------------
tim_proc:
	CW	r1, 0
	JES	done
	AWT	r1, -1
	IN	r5, 0\2 + mx_chan\14	; reset MULTIX
	.word	fail, fail, ok, fail
fail:	HLT	040
ok:
done:	LIP

; ------------------------------------------------------------------------
mx_proc:
	CW	r1, 0		; MULTIX interrupt should come after r1 counter drops to 0
	JN	too_early	; MULTIX interrupt came too early
	LW	r4, [stackp]
	LW	r4, [r4-1]
	CW	r4, iwyze
	JN	int_fail
	HLT	077
int_fail:
	HLT	041
too_early:
	HLT	042

; ------------------------------------------------------------------------
start:
	LW	r3, stack
	RW	r3, stackp
	LW	r3, mx_proc
	RW	r3, int_mx
	LW	r3, tim_proc
	RW	r3, int_tim
	RW	r3, int_ex
	LW	r1, 100		; reset counter: wait 100 timer interrupts (100*10ms = 1s)

	IM	mask
loop:	HLT
	UJS	loop
	HLT	050

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT r1 : 0
; XPCT ir&0x3f : 0o77
