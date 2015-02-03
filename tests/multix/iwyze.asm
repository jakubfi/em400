; OPTS -c configs/multix.cfg

; Check if all MULTIX reset triggers work

	.equ	stackp 0x61
	.equ	prog_beg 0x70
	.equ	int_mx 0x40 + 12 + 1
	.equ	unmask_chan 0b0000011110000000
	.equ	iwyze 0b0000001000000000
	.equ	mx_chan 1

	UJ	start

mask:
	.word	unmask_chan
mx_proc:
	LW	r4, [stackp]
	LW	r4, [r4-1]
	CW	r4, iwyze
	JN	int_fail
	AW	r3, 1		; increase interrupt counter
	LIP
int_fail:
	HLT	041

	.org	prog_beg
start:
	LW	r3, stack
	RW	r3, stackp
	LW	r3, mx_proc
	RW	r3, int_mx
	LWT	r3, 0		; reset interrupt counter

	; IWYZE by MULTIX initialization
	IM	mask
	HLT

	; IWYZE by MCL
	MCL
	IM	mask
	HLT

	; IWYZE, by software MULTIX reset
	IN	r5, mx_chan\14
	.word	fail, fail, ok, fail
ok:	HLT
	HLT	077

fail:	HLT	040

stack:

; XPCT int(rz[15]) : 0
; XPCT int(rz[6]) : 0
; XPCT int(alarm) : 0
; XPCT int(r3) : 3
; XPCT oct(ir[10-15]) : 077
