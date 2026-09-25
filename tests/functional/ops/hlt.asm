; PRECMD clock on

	.include cpu.inc

	lw	r1, stack
	rw	r1, STACKP
	lw	r1, fin
	rw	r1, INTV_TIMER
	im	mask
loop:	hlt
	ujs	loop

fin:	hlt	077
mask:	.word	IMASK_GROUP_H
stack:


; XPCT sr : 0
