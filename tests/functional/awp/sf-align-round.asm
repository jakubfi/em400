
	.include awp-fp.inc

operation:
	lf	r7+ARG1
	sf	r7+ARG2
	uj	r5

tests:
	.word	0, ?C, 0
	.word	0x6000, 0x0000, 0x0002
	.word	0xb000, 0x0000, 0x00dd
	.word	0x6000, 0x0000, 0x0302
fin:
