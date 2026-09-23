	lw	r4, tests
loop:
	lw	r0, [r4]
	lw	r1, [r4+1]

	ngc	r1

	cw	r0, [r4+2]
	jn	err_r0
	cw	r1, [r4+3]
	jn	err_r1

	aw	r4, 4
	cw	r4, tests_end

	jn	loop

	hlt	077
err_r0:
	hlt	040
err_r1:
	hlt	041

tests:
	; flags in, arg, flags out, result
	.word	0,	0,	?M,	-1
	.word	?C,	0,	?ZC,	0
	.word	0,	1,	?M,	-2
	.word	?C,	1,	?M,	-1
	.word	0,	32767,	?M,	-32768
	.word	?C,	32767,	?M,	-32767
	.word	0,	-32768,	0,	32767
	.word	?C,	-32768,	?V,	-32768
tests_end:

; XPCT ir : 0xec3f
