; OPTS -c configs/multix.cfg

; Test "REQUEUE INTERRUPT" command
; The only thing we can test here is if MULTIX acknowledges the command.
; Everything else happens under the blanket and cannot be verified from CPU perspective

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

	IM	mask
	HLT		; wait for mx int

repeat:	IN	r5, 0b001\2 + 0\10 + mx_chan\14
	.word	fail, repeat, ok, fail
fail:	HLT	041
ok:	HLT	077

stack:

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT ir : 0xec3f
