	lw	r4, tests
loop:
	lw	r0, [r4]
	lw	r1, [r4+1]

	sw	r1, [r4+2]

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
	.word	0,	10,	100,	?M,	-90
	.word	0,	1,	0,	?C,	1
	.word	0,	-1,	1,	?MC,	-2
	.word	0,	-32768,	1,	?MVC,	32767
	.word	0,	0,	-32768,	?V,	-32768
	.word	0,	1,	-32768,	?V,	-32767
	.word	0,	-1,	-32768,	?C,	32767
	.word	0,	32767,	-32768,	?V,	-1
	.word	0,	-32768,	-32768,	?ZC,	0
	.word	?ZMVC,	0,	-32768,	?V,	-32768
	.word	?ZMVC,	1,	-32768,	?V,	-32767
	.word	?ZMVC,	-1,	-32768,	?VC,	32767
	.word	?ZMVC,	32767,	-32768,	?V,	-1
	.word	?ZMVC,	-32768,	-32768,	?ZVC,	0
	.word	?ZMVCLEGYX1234567,	3,	1,	?VCLEGYX1234567,	2
tests_end:
