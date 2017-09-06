	lw	r7, -10_000

	ld	d10m
loop:
	sd	d10
	sd	d10
	sd	d10
	sd	d10
	sd	d10
	sd	d10
	sd	d10
	sd	d10
	sd	d10
	sd	d10
	sd	d10
	sd	d10
	sd	d10
	sd	d10
	sd	d10
	irb	r7, loop
	ujs	loop

	hlt	077

d10m:	.dword	10_000_000
d10:	.dword	10
