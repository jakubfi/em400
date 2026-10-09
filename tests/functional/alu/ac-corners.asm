	lw	r4, tests
loop:
	lw	r0, [r4]
	lw	r1, [r4+1]

	ac	r1, [r4+2]

	cw	r0, [r4+3]
	jn	err_r0
	cw	r1, [r4+4]
	jn	err_r1

	aw	r4, 5
	cw	r4, tests_end

	jn	loop

	hlt	077
err_r0:
	hlt	040
err_r1:
	hlt	041

tests:
	; flags in, arg1, arg2, flags out, result
	.word	0,	32767,	0,	0,	32767
	.word	?C,	10,	100,	0,	111
	.word	?C,	-2,	0,	?M,	-1
	.word	?C,	-1,	0,	?ZC,	0
	.word	?C,	-32768,	32767,	?ZC,	0
	.word	?C,	32767,	0,	?V,	-32768
	.word	?C,	-32768,	-32768,	?MVC,	1
	.word	?C,	-1,	-1,	?MC,	-1
	.word	?C,	32767,	32767,	?V,	-1
	.word	?V,	0,	1,	?V,	1
	.word	?VC,	0,	1,	?V,	2
	.word	?ZMVCLEGYX1234567,	1,	1,	?VLEGYX1234567,	3
tests_end:
