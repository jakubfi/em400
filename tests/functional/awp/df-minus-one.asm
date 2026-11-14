
; DF by a -1.0 mantissa: with a negative dividend the partial remainder can
; reach exactly +1.0, which the hardware takes as negative (T0 picks add/sub),
; so the quotient comes out 1 ulp low. Not every such dividend is hit.

	.include awp-fp.inc

operation:
	lf	r7+ARG1
	df	r7+ARG2
	uj	r5

tests:
	.word	0, 0, 0
	.word	0x8000, 0x0000, 0x0f10 ; -65535.9999982
	.word	0x8000, 0x0000, 0x00f8 ; -2^-8
	.word	0x7fff, 0xffff, 0xf018 ; 16777215.9995, 1 ulp low

	.word	0, 0, 0
	.word	0xba7d, 0x8000, 0x00f1 ; -1.65724195539951e-05
	.word	0x8000, 0x0000, 0x00e2 ; -2^-30
	.word	0x4582, 0x7fff, 0xff0f ; 17794.4999999, 1 ulp low

	.word	0, 0, 0
	.word	0x952d, 0x2089, 0xbc00 ; -0.834560330149543
	.word	0x8000, 0x0000, 0x00d4 ; -2^-44
	.word	0x6ad2, 0xdf76, 0x432c ; 14681740593248, 1 ulp low

	.word	0, 0, 0
	.word	0xb000, 0x0000, 0x0000 ; -0.625
	.word	0x8000, 0x0000, 0x0000 ; -1.0
	.word	0x4fff, 0xffff, 0xff00 ; 0.625, 1 ulp low

	.word	0, 0, 0
	.word	0xa000, 0x0000, 0x0000 ; -0.75
	.word	0x8000, 0x0000, 0x0000 ; -1.0
	.word	0x6000, 0x0000, 0x0000 ; 0.75, exact

	.word	0, ?M, 0
	.word	0x6000, 0x0000, 0x0000 ; 0.75
	.word	0x8000, 0x0000, 0x0000 ; -1.0
	.word	0xa000, 0x0000, 0x0000 ; -0.75, exact
fin:
