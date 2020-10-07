
	ld	a
	dw	b
	cw	r1, [regs]
	jn	fail
	cw	r2, [regs+1]
	jn	fail
	hlt	077

fail:
	hlt	040

a:	.dword	2147483647
b:	.word	-32768
regs:	.word	32767, 1

; XPCT ir : 0xec3f
