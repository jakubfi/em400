
	.include awp-dword.inc

operation:
	ld	r7+ARG1
	dw	r7+ARG2
	uj	r5

tests:
	.word	?C, ?C, 0
	.dword	15
	.word	3, 0
	.word	0, 5

	.word	?V, ?V, 0
	.dword	15
	.word	3, 0
	.word	0, 5
fin:
