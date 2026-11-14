
	.include awp-fp.inc

operation:
	lf	r7+ARG1
	af	r7+ARG2
	uj	r5

tests:
	.word	0, 0, I_FP_UF
	.word	0x4000, 0x0000, 0x0181
	.word	0x8000, 0x0000, 0x0080
	.word	0x4000, 0x0000, 0x005b ; exponent -165, wrapped
fin:
