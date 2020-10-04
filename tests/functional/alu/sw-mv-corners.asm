	lw	r4, tests
loop:
	lw	r0, [r4]
	lw	r1, [r4+1]
	lw	r2, [r4+2]

	sw	r1, r2

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
	.word 0, 0, -32768
	.word ?V, -32768

	.word 0, 1, -32768
	.word ?V, -32767

	.word 0, -1, -32768
	.word ?C, 32767

	.word 0, 32767, -32768
	.word ?V, -1

	.word 0, -32768, -32768
	.word ?ZC, 0

	.word ?ZMVC, 0, -32768
	.word ?V, -32768

	.word ?ZMVC, 1, -32768
	.word ?V, -32767

	.word ?ZMVC, -1, -32768
	.word ?VC, 32767

	.word ?ZMVC, 32767, -32768
	.word ?V, -1

	.word ?ZMVC, -32768, -32768
	.word ?ZVC, 0
tests_end:

; XPCT ir : 0xec3f
