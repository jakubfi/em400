.prog "multix/IWYZE"

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
	IM mask				; first IWYZE, by MULTIX initialization
	MCL
	IM mask				; second IWYZE, by MCL
	IN r5, 0b0000000000000110	; third IWYZE, by software reset
	.data fail, fail, fin, fail

fin:
	LW r5, 13
fail:
	hlt 077

.finprog

; XPCT int(rz[15]) : 0
; XPCT int(rz[6]) : 0
; XPCT int(alarm) : 0
; XPCT int(r3) : 3
; XPCT bin(r4) : 0b0000001000000000
; XPCT int(r5) : 13
