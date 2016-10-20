	lw r7, -10000

	ld d2m
loop:
	dw d2
	dw d2
	dw d2
	dw d2
	dw d2
	dw d2
	dw d2
	dw d2
	dw d2
	dw d2
	dw d2
	dw d2
	dw d2
	dw d2
	dw d2
	irb r7, loop
	ujs loop

	hlt 077

d2m:	.dword 2000000
d2:	.dword 2
