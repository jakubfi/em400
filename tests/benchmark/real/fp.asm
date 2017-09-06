	lw	r7, -10_000

	lf	d1m
loop:
	af	d100
	af	d100
	af	d100
	sf	d100
	af	d100
	sf	d100
	af	d100
	sf	d100
	df	d100
	mf	d100
	mf	d0125
	df	d0125
	mf	d0125
	af	d1m
	irb	r7, loop
	ujs	loop

	hlt	077

d1m:	.float	1000000000.0
d100:	.float	-100
d0125:	.float	0.125
