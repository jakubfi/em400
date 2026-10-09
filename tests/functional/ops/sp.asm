	.include cpu.inc

	lw	r1, stack
	rw	r1, STACKP
	lw	r1, proc
	rw	r1, INTV_SW_L

	; sp should load IC, R0 and SR from the context
	; sp should apply the new interrupt mask at once: pending soft L gets served
	; before the process runs any instruction
	sil
	lwt	r0, -1
	sp	ctx
	hlt	040

proc:	hlt	077

	; RM: parity, CPU high, channels 0-1, channels 4-9, group L; Q=0, BS=1, NB=10
ctx:	.word	process, 0b0101101000111100, 0b1010010101011010

	.org	80
process:
	hlt	040

	.org	100
stack:

; XPCT [100] : 80
; XPCT [101] : 0b0101101000111100
; XPCT [102] : 0b1010010101011010
; XPCT rz[31] : 0
