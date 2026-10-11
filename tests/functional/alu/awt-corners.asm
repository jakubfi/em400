	lw	r4, tests_dec
loop_dec:
	lw	r0, [r4]
	lw	r1, [r4+1]

	awt	r1, -1

	cw	r0, [r4+2]
	jn	err_r0
	cw	r1, [r4+3]
	jn	err_r1

	aw	r4, 4
	cw	r4, tests_dec_end

	jn	loop_dec

	lw	r4, tests_inc
loop_inc:
	lw	r0, [r4]
	lw	r1, [r4+1]

	awt	r1, 1

	cw	r0, [r4+2]
	jn	err_r0
	cw	r1, [r4+3]
	jn	err_r1

	aw	r4, 4
	cw	r4, tests_inc_end

	jn	loop_inc

	hlt	077
err_r0:
	hlt	040
err_r1:
	hlt	041

tests_dec:
	; flags in, arg, flags out, result
	.word	0,	0,	?M,	-1
	.word	0,	1,	?ZC,	0
	.word	0,	2,	?C,	1
	.word	0,	-1,	?MC,	-2
	.word	0,	32767,	?C,	32766
	.word	0,	-32768,	?MVC,	32767
	.word	?V,	5,	?VC,	4
	.word	?ZMVCLEGYX1234567,	5,	?VCLEGYX1234567,	4
tests_dec_end:

tests_inc:
	; flags in, arg, flags out, result
	.word	0,	0,	0,	1
	.word	0,	-1,	?ZC,	0
	.word	0,	-2,	?M,	-1
	.word	0,	-32768,	?M,	-32767
	.word	0,	32766,	0,	32767
	.word	0,	32767,	?V,	-32768
	.word	?V,	5,	?V,	6
	.word	?ZMVCLEGYX1234567,	5,	?VLEGYX1234567,	6
tests_inc_end:
