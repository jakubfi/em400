; ------------------------------------------------------------------------
; fill memory
fill_7:
	.res	1
	lw	r6, test_start
next_7:
	lw	r2, r6
	mw	mul
	rw	r2, r6
	irb	r6, next_7
	uj	[fill_7]
mul:	.word   7

; vim: tabstop=8 shiftwidth=8 autoindent syntax=emas
