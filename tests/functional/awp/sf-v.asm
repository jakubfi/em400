
	.include awp-fp.inc

operation:
	lf	r7+ARG1
	sf	r7+ARG2
	uj	r5

tests:
	.word	?V, ?ZV, 0
	.word	0x4000, 0x0000, 0x0001
	.word	0x4000, 0x0000, 0x0001
	.word	0x0000, 0x0000, 0x0000
fin:
