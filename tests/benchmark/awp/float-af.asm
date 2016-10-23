	lw r7, -10000

	lf d1m
loop:
	af d100
	af d100
	af d100
	af d100
	af d100
	af d100
	af d100
	af d100
	af d100
	af d100
	af d100
	af d100
	af d100
	af d100
	af d100
	irb r7, loop
	ujs loop

	hlt 077

d1m:	.float 1000000000
d100:	.float 100
