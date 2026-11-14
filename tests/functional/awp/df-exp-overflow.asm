
	.include awp-fp.inc

operation:
	lf	r7+ARG1
	df	r7+ARG2
	uj	r5

tests:
	.word	0, 0, I_FP_OF
	.word	0x7fff, 0xffff, 0x007f
	.word	0x4000, 0x0000, 0x0080
	.word	0x7fff, 0xffff, 0x0000 ; exponent 256, wrapped
fin:
