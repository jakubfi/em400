	lw	r7, -10_000

	ld	d2m
loop:
	dw	d2
	dw	d2
	dw	d2
	dw	d2
	dw	d2
	dw	d2
	dw	d2
	dw	d2
	dw	d2
	dw	d2
	dw	d2
	dw	d2
	dw	d2
	dw	d2
	dw	d2
	irb	r7, loop
	ujs	loop

	hlt	077

d2m:	.dword	2_000_000
d2:	.dword	2
