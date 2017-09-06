; test including this framework has to define:
;  * tests - beginning of test data
;  * fin - end of test data
;  * operation - data load and arithmetic operation, then "uj r5". r7 = pointer to current test data

	.const	INT_ILL_OPCODE 1\6
	.const	INT_DIV_OF 1\7
	.const	INT_FP_UF 1\8
	.const	INT_FP_OF 1\9
	.const	INT_DIV0 1\10
	.const	INT_ENORM INT_DIV0

; each test is 9 words long and contains:

	.const	R0_IN 0
	.const	R0_OUT 1
	.const	INT_OUT 2
	.const	ARG1 3
	.const	ARG2 6
	.const	RES 9
	.const	TEST_LEN 12

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
	cw	r3, [r7+RES+2]
	jn	exit_fail
	ki	intbuf
	lw	r4, INT_ILL_OPCODE | INT_DIV_OF | INT_FP_UF | INT_FP_OF | INT_DIV0
	nm	r4, intbuf
	lw	r4, [intbuf]
	cw	r4, [r7+INT_OUT]
	jn	exit_fail

	awt	r7, TEST_LEN
	cw	 r7, fin
	je	 exit_ok
	uj	 loop

intbuf:	.res	 1
inizer:	.word	 0

exit_fail:
	hlt	 040
exit_ok:
	hlt	 077

; vim: tabstop=8 shiftwidth=8 autoindent syntax=emas
