
	.include awp-fp.inc

operation:
	lf	r7+ARG1
	nrf
	uj	r5

tests:
	.word	0, 0, I_FP_UF
	.word	0x0001, 0x0000, 0x0080 ; 2^-15 * 2^-128 (denormalized)
	.float	0
	.word	0x4000, 0x0000, 0x0072 ; exponent -142, wrapped

	.word	0, 0, I_FP_UF
	.word	0x0000, 0x0001, 0x0081 ; 2^-31 * 2^-127 (denormalized)
	.float	0
	.word	0x4000, 0x0000, 0x0063 ; exponent -157, wrapped
fin:
