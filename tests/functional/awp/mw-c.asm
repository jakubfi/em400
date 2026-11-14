
	.include awp-dword.inc

operation:
	lw	r2, [r7+ARG1]
	mw	r7+ARG2
	uj	r5

tests:
	.word	?C, ?C, 0
	.word	5, 0
	.word	3, 0
	.dword	15
fin:
