
	.include awp-fp.inc

operation:
	lf	r7+ARG1
	sf	r7+ARG2
	uj	r5

tests:
	.word	0, 0, I_FP_OF
	.word	0x7fff, 0xffff, 0x007f
	.word	0x8000, 0x0000, 0x007f
	.word	0x7fff, 0xffff, 0x8080 ; exponent 128, wrapped
fin:
