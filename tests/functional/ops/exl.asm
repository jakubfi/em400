
	.include cpu.inc

	uj	start

	.res	128
stack:	.res	16
mask:	.word	IMASK_CH2_3 | IMASK_CH4_9 | IMASK_CH10_15 | IMASK_GROUP_L

start:
	lw	r0, 0xfafa
	lw	r1, exlp
	rw	r1, EXLV

	lw	r1, stack
	rw	r1, STACKP

	im	mask

	exl	23
	hlt	040

exlp:
	hlt	077

; XPCT rz[6] : 0
; XPCT ir : 0xec3f

; new process vector

; XPCT sr : 0b0000001110000000
; XPCT r0 : 0

; new stack pointer

; XPCT [97] : 134

; stack contents

; XPCT [130] : 160
; XPCT [131] : 0xfafa
; XPCT [132] : 0b0000001111000000
; XPCT [133] : 23
