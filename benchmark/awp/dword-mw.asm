	lw r7, -10000

	lw r2, d10
loop:
	mw d2
	mw d2
	mw d2
	mw d2
	mw d2
	mw d2
	mw d2
	mw d2
	mw d2
	mw d2
	mw d2
	mw d2
	mw d2
	mw d2
	mw d2
	irb r7, loop

	hlt 077

d10:	.dword 10
d2:	.dword 2
