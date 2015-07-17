	lw r7, -10000

	lf d1m
loop:
	sf d100
	sf d100
	sf d100
	sf d100
	sf d100
	sf d100
	sf d100
	sf d100
	sf d100
	sf d100
	sf d100
	sf d100
	sf d100
	sf d100
	sf d100
	irb r7, loop

	hlt 077

d1m:	.float 1000000000
d100:	.float 100
