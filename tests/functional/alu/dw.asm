
	.include awp-dword.inc

operation:
	ld r7+ARG1
	dw r7+ARG2
	uj r5

tests:
	.word  0, 0, 0
	.dword 5
	.word  2, 0
	.word  1, 2

	.word  0, 0, 0
	.dword 4
	.word  2, 0
	.word  0, 2

	.word  0, 0, 0
	.dword 39746371
	.word  1213, 0
	.word  0, 0b0111111111111111

	.word  0, ?M, 0
	.dword 19
	.word  -4, 0
	.word  3, -4

	.word  0, ?M, 0
	.dword -33
	.word  5, 0
	.word  -3, -6

	.word  0, ?Z, 0
	.dword 0
	.word  22, 0
	.word  0, 0

	.word  0, ?M, 0
	.dword -39746371
	.word  1213, 0
	.word  0, 0b1000000000000001
fin:

; XPCT sr : 0
; XPCT ir : 0xec3f
