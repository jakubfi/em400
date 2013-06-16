.prog "multix/IWYZE-poweron"

; CONFIG configs/multix.cfg

	.equ stackp 0x61
	.equ stack 0x64
	.equ prog_beg 0x70

	.equ ints 0x40
	.equ intsc ints + 12
	.equ int_c3 intsc + 3

	.equ unmask_chan 0b0000011110000000

	UJ start

	.ic stackp
	.data stack
	.ic int_c3
	.data mx_int

	.ic prog_beg
mask:
	.data unmask_chan
mx_int:
	AW r3, 1
	LW r4, [stackp]
	LW r4, [r4-1]
	LIP

start:
	LWT r3, 0
	IM mask

	hlt 077

.finprog

; XPCT int(rz[15]) : 0
; XPCT int(rz[6]) : 0
; XPCT int(alarm) : 0
; XPCT int(r3) : 1
; XPCT bin(r4) : 0b0000001000000000
