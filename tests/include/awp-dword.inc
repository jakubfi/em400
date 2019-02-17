; test including this framework has to define:
;  * tests - beginning of test data
;  * fin - end of test data
;  * operation - data load and arithmetic operation, then "uj r5". r7 = pointer to current test data

	.include cpu.inc

; each test is 9 words long and contains:

	.const	R0_IN 0
	.const	R0_OUT 1
	.const	INT_OUT 2
	.const	ARG1 3
	.const	ARG2 5
	.const	RES 7
	.const	TEST_LEN 9

	lw	r7, tests
	lwt	r6, 0
loop:
	awt	r6, 1

	lw	r0, [r7+R0_IN]
	fi	inizer
	rj	r5, operation

	cw	r0, [r7+R0_OUT]
	jn	exit_fail
	cw	r1, [r7+RES]
	jn	exit_fail
	cw	r2, [r7+RES+1]
	jn	exit_fail
	ki	intbuf
	lw	r4, I_ILLEGAL | I_DIV_OF | I_FP_UF | I_FP_OF | I_DIV0
	nm	r4, intbuf
	lw	r4, [intbuf]
	cw	r4, [r7+INT_OUT]
	jn	exit_fail

	awt	r7, TEST_LEN
	cw	r7, fin
	je	exit_ok
	uj	loop

intbuf:	.res	1
inizer:	.word	0

exit_fail:
	hlt	040
exit_ok:
	hlt	077

; vim: tabstop=8 shiftwidth=8 autoindent syntax=emas
