	lw r7, -10000

	lf d1m
loop:
	af d100
	af d100
	af d100
	sf d100
	af d100
	sf d100
	af d100
	sf d100
	df d100
	mf d100
	mf d0125
	df d0125
	mf d0125
	af d1m
	irb r7, loop

	hlt 077

d1m:	.word 0x7735, 0x9400, 0x001e ; 1000000000
d100:	.word 0x9c00, 0x0000, 0x0007 ; -100
d0125:	.word 0x4000, 0x0000, 0x00fe ; 0.125
