	lw r7, -10000

	ld d100
loop:
	ad d10
	ad d10
	ad d10
	ad d10
	ad d10
	ad d10
	ad d10
	ad d10
	ad d10
	ad d10
	ad d10
	ad d10
	ad d10
	ad d10
	ad d10
	irb r7, loop
	ujs loop

	hlt 077

d100:	.dword 100
d10:	.dword 10
