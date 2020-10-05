; OUTPUT Word

	rws	r4, return

	lw	r6, 0		; start from the value 0
loop:
	lw	r0, r6		; load current value
	srz	r0		; shift it
	rw	r0, r7		; store the result

	lw	r1, r7		; run crc
	lwt	r2, 1
	exl	1

	irb	r6, loop	; loop over

	rw	r1, r7		; store the final crc

	lws	r4, return	; load return address and end the test
	uj	r4

return:	.res	1

data:
