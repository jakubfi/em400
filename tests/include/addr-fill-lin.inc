; ------------------------------------------------------------------------
; fill memory
fill_lin:
	.res	1
	lw	r1, test_start
next_lin:
	rw	r1, r1
	irb	r1, next_lin
	uj	[fill_lin]

; vim: tabstop=8 shiftwidth=8 autoindent syntax=emas
