; SF keeps the borrow from the subtrahend's bits shifted out during
; alignment, so the result and C come from the exact difference.

	.include awp-fp.inc

operation:
	lf	r7+ARG1
	sf	r7+ARG2
	uj	r5

tests:
	; exponent gap 27: the borrow keeps the result from rounding up
	.word	0, 0, 0
	.word	0x7fff, 0xffff, 0xf3ed ; 1.9073486327674e-06
	.word	0x8000, 0x0000, 0x08d2 ; -1.42108547149952e-14
	.word	0x4000, 0x0007, 0xf9ee ; 1.90734864697478e-06

	; exponent gap 36: the borrow sets C
	.word	0, ?C, 0
	.word	0x4427, 0xb2d0, 0x9816 ; 2233305.40740967
	.word	0x4000, 0x0000, 0x06f2 ; 3.05175781256661e-05
	.word	0x4427, 0xb2d0, 0x9416 ; 2233305.40737915

	; exponent gap 25: the borrow sets C
	.word	0, ?M | ?C, 0
	.word	0x9967, 0x5261, 0xc007 ; -102.596399202943
	.word	0xa8f0, 0x0000, 0x01ee ; -2.59466469287178e-06
	.word	0x9967, 0x528d, 0x4807 ; -102.596396608278
fin:
