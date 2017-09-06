	lw	r7, -10_000

	lf	d1m
loop:
	af	d100
	af	d100
	af	d100
	af	d100
	af	d100
	af	d100
	af	d100
	af	d100
	af	d100
	af	d100
	af	d100
	af	d100
	af	d100
	af	d100
	af	d100
	irb	r7, loop
	ujs	loop

	hlt	077

d1m:	.float 1_000_000_000
d100:	.float 100
