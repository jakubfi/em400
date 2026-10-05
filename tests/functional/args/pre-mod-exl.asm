; pre-modification affects EXL byte argument
; full 16-bit premodified argument is stored on the stack

	.include cpu.inc

	uj	start

	.org	OS_START
start:
	lw	r1, exl_proc
	rw	r1, EXLV
	lw	r1, stack
	rw	r1, STACKP

	md	1
	exl	0x3f	; arg = 0x40
	cw	r2, 0x40
	jes	1
	hlt	040

	md	0xff
	exl	0x40	; arg = 0x13f
	cw	r2, 0x13f
	jes	1
	hlt	041

	md	0x1200
	exl	0x34	; arg = 0x1234
	cw	r2, 0x1234
	jes	1
	hlt	042

	md	0xffff
	exl	2	; arg = 1
	cw	r2, 1
	jes	1
	hlt	043

	hlt	077

exl_proc:
	lw	r1, [STACKP]
	lw	r2, [r1-1]
	lip

stack:
