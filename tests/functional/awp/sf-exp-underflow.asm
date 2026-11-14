
	.include awp-fp.inc

operation:
	lf	r7+ARG1
	sf	r7+ARG2
	uj	r5

tests:
	.word	0, 0, I_FP_UF
	.word	0x4000, 0x0000, 0x0180
	.word	0x4000, 0x0000, 0x0080
	.word	0x4000, 0x0000, 0x005a ; exponent -166, wrapped
fin:
