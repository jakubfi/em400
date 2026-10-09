	lw	r4, tests
loop:
	lw	r0, [r4]
	lw	r1, [r4+1]

	aw	r1, [r4+2]

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
	.word	0,	1,	1,	0,	2
	.word	0,	0,	0,	?Z,	0
	.word	0,	-1,	0,	?M,	-1
	.word	0,	0,	-1,	?M,	-1
	.word	0,	-1,	1,	?ZC,	0
	.word	0,	-1,	32767,	?C,	32766
	.word	0,	-1,	-1,	?MC,	-2
	.word	0,	-32768,	32767,	?M,	-1
	.word	0,	32767,	1,	?V,	-32768
	.word	0,	32767,	32767,	?V,	-2
	.word	0,	-1,	-32768,	?MVC,	32767
	.word	0,	-32768,	-32768,	?MVC,	0
	.word	?V,	0,	1,	?V,	1
	.word	?ZMVCLEGYX1234567,	1,	1,	?VLEGYX1234567,	2
tests_end:
