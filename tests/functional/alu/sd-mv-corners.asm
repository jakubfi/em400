	lw	r4, tests
loop:
	lw	r0, [r4]
	ld	r4+1

	sd	r4+3

	cw	r0, [r4+5]
	jn	err_r0
	cw	r1, [r4+6]
	jn	err_res
	cw	r2, [r4+7]
	jn	err_res

	aw	r4, 8
	cw	r4, tests_end

	jn	loop

	hlt	077
err_r0:
	hlt	040
err_res:
	hlt	041

tests:
	.word	0
	.dword	0, -2147483648
	.word	?V
	.dword	-2147483648

	.word	0
	.dword	1, -2147483648
	.word	?V
	.dword	-2147483647

	.word	0
	.dword	-1, -2147483648
	.word	?C
	.dword	2147483647

	.word	0
	.dword	2147483647, -2147483648
	.word	?V
	.dword	-1

	.word	0
	.dword	-2147483648, -2147483648
	.word	?ZC
	.dword	0
tests_end:

; XPCT ir : 0xec3f
