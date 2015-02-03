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
	HLT	041

	.org	prog_beg
start:
	LW	r3, stack
	RW	r3, stackp
	LW	r3, mx_proc
	RW	r3, int_mx

	OU	r5, 0b101\2 + mx_chan\14
	.word	fail, ok, fail, fail
ok:	HLT	077
fail:	HLT	040

stack:

; XPCT int(rz[15]) : 0
; XPCT int(rz[6]) : 0
; XPCT int(alarm) : 0
; XPCT int(r3) : 3
; XPCT oct(ir[10-15]) : 077
