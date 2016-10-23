; OPTS -c configs/multix.cfg

; Check if MULTIX rejects commands before initialization completes

	.equ	stackp 0x61
	.equ	prog_beg 0x70
	.equ	int_mx 0x40 + 12 + 1
	.equ	unmask_chan 0b0000011110000000
	.equ	mx_chan 1

	UJ	start

mask:
	.word	unmask_chan
mx_proc:
	HLT	041		; test should end before MX int

	.org	prog_beg
start:
	LW	r3, stack
	RW	r3, stackp
	LW	r3, mx_proc
	RW	r3, int_mx
	IM	mask

	OU	r5, 0b101\2 + mx_chan\14
	.word	fail, ok, fail, fail
ok:	HLT	077	; EN = command rejected -> this is OK
fail:	HLT	040	; NO, OK, PE -> this is bad

stack:

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT ir&0x3f : 0o77
