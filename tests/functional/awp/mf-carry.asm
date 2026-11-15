; MF sets C from M-1 of the exact two's complement product. A negative
; product with lost low bits differs from the negated truncated magnitude.

	.include awp-fp.inc

operation:
	lf	r7+ARG1
	mf	r7+ARG2
	uj	r5

tests:
	.word	0, ?M | ?C, 0
	.word	0x7fff, 0xffff, 0xf1fa ; 0.0156249999995737
	.word	0x8000, 0x0000, 0x0d07 ; -127.999999996973
	.word	0x8000, 0x0000, 0x1c01 ; -1.99999999989814

	.word	0, ?M | ?C, 0
	.word	0xbfff, 0xffff, 0xffe7 ; -1.49011611939019e-08
	.word	0x4000, 0x0000, 0xb3e6 ; 7.45058060177563e-09
	.word	0xbfff, 0xffff, 0x4ccc ; -1.11022302535217e-16

	; mera400f only: M-1 is 0 in the exact product, so no rounding up
	.word	0, ?M, 0
	.word	0x8719, 0x95e9, 0xd117 ; -7923306.08665466
	.word	0x62f1, 0x3b41, 0x5de2 ; 7.19900056825353e-10
	.word	0xa28b, 0xbcfb, 0x54f9 ; -0.00570398850203446
fin:
