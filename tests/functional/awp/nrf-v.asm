
	.include awp-fp.inc

operation:
	lf	r7+ARG1
	nrf
	uj	r5

tests:
	.word	?V, ?V, 0
	.word	0x4000, 0x0000, 0x0001
	.float	0
	.word	0x4000, 0x0000, 0x0001
fin:
