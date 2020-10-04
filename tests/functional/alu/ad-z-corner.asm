
	lw	r0, [r0_set]
	ld	a

	ad	b

	cw	r0, [r0_res]
	jn	fail_r0
	cw	r1, [res]
	jn	fail_res
	cw	r2, [res+1]
	jn	fail_res

	hlt	077
fail_r0:
	hlt	040
fail_res:
	hlt	041

r0_set:	.word	0
a:	.dword	-2147483648
b:	.dword	-2147483648
r0_res:	.word	?MVC
res:	.dword	0

; XPCT ir : 0xec3f
