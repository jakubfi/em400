; OPTS -c configs/multix.cfg

; MULTIX' "check if exists" command should return OK both before and after mx initialization

	.equ	stackp 0x61
	.equ	prog_beg 0x70
	.equ	int_mx 0x40 + 12 + 1
	.equ	unmask_chan 0b0000010000000000
	.equ	mx_chan 1
	.equ	iwyze 0b0000001000000000

	UJ	start

mask:
	.word	unmask_chan
mx_proc:
	LW	r4, [stackp]
	LW	r4, [r4-1]
	CW	r4, iwyze       ; is it IWYZE?
	BB	r0, ?E
	HLT	042		; not iwyze
	LIP

	.org	prog_beg
start:
	LW	r3, stack
	RW	r3, stackp
	LW	r3, mx_proc
	RW	r3, int_mx

	; "exists" before initialization
	IN	r5, 0b10\4 + mx_chan\14
	.word	fail, fail, ok, fail
fail:	HLT	040
ok:	IM	mask
	HLT		; wait for mx int

	; "exists" after initialization
	IN	r5, 0b10\4 + mx_chan\14
	.word	fail2, fail2, ok2, fail2
fail2:	HLT	041
ok2:	HLT	077

stack:

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT r4 : 0b0000001000000000
; XPCT ir : 0xec3f
