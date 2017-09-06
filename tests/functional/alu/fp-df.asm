
	.include awp-fp.inc

operation:
	lf	r7+ARG1
	df	r7+ARG2
	uj	r5

tests:
	.word	0, ?M, 0
	.float	1000000000
	.float	-100
	.float	-10000000

	.word	0, ?M, 0
	.float	0.125
	.float	-100
	.word	0xae14, 0x7ae1, 0x48f7

	.word	0, 0, 0
	.float	2
	.float	0.125
	.float	16
fin:

; XPCT sr : 0
; XPCT ir : 0xec3f
