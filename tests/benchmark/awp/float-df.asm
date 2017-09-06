	lw	r7, -10_000

	lf	d100
loop:
	df	d1p3
	df	d1p3
	df	d1p3
	df	d1p3
	df	d1p3
	df	d1p3
	df	d1p3
	df	d1p3
	df	d1p3
	df	d1p3
	df	d1p3
	df	d1p3
	df	d1p3
	df	d1p3
	df	d1p3
	irb	r7, loop
	ujs	loop

	hlt	077

d100:	.float	100
d1p3:	.float	1.3
