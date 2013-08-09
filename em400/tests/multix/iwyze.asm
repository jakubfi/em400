.prog "multix/IWYZE"

; CONFIG configs/multix.cfg

	.equ stackp 0x61
	.equ prog_beg 0x70
	.equ int_mx 0x40 + 12 + 1
	.equ unmask_chan 0b0000011110000000
	.equ iwyze 0b0000001000000000
	.equ mx_chan 1

	UJ start

	.ic prog_beg
mask:
	.data unmask_chan
mx_proc:
	LW r4, [stackp]
	LW r4, [r4-1]
	CW r4, iwyze
	JN int_exit
	AW r3, 1
int_exit:
	LIP

start:
	LW r3, stack
	RW r3, stackp
	LW r3, mx_proc
	RW r3, int_mx
	LWT r3, 0

	IM mask				; first IWYZE, by MULTIX initialization
	MCL
	IM mask				; second IWYZE, by MCL
	IN r5, mx_chan\14		; third IWYZE, by software reset
	.data fail, fail, fin, fail

fin:
	LW r5, 13
fail:
	hlt 077

stack:
	.res 4

.finprog

; XPCT int(rz[15]) : 0
; XPCT int(rz[6]) : 0
; XPCT int(alarm) : 0
; XPCT int(r3) : 3
; XPCT int(r5) : 13
