
	.include awp-dword.inc

operation:
	ld r7+ARG1
	sd r7+ARG2
	uj r5

tests:
	.word  0, 0, 0
	.dword 573447
	.dword -638979
	.dword 1212426

	.word 0, ?V, 0
	.word 0b0100000000000000, 0b0000000000000000
	.word 0b1100000000000000, 0b0000000000000000
	.word 0b1000000000000000, 0b0000000000000000

	.word ?V, ?MVC, 0
	.word 0b1100000000000000, 0b0000000000000000
	.word 0b0100000000000000, 0b0000000000000000
	.word 0b1000000000000000, 0b0000000000000000

	.word  0, ?ZC, 0
	.dword -1
	.dword -1
	.dword 0
fin:

; XPCT int(sr) : 0
; XPCT oct(ir[10-15]) : 077

