
	.include awp-dword.asm

operation:
	lw r2, [r7+ARG1]
	mw r7+ARG2
	uj r5

tests:
	.word  ?V, ?ZV, 0
	.word  -9999, 0
	.word  0, 0
	.dword 0

	.word  ?ZV, ?ZV, 0
	.word  9999, 0
	.word  0, 0
	.dword 0

	.word  ?ZV, ?V, 0
	.word  2, 0
	.word  2, 0
	.dword 4

	.word  ?V, ?V, 0
	.word  -2, 0
	.word  -2, 0
	.dword 4

	.word  ?V, ?MV, 0
	.word  -2, 0
	.word  4, 0
	.dword -8

	.word  ?MV, ?MV, 0
	.word  2, 0
	.word  -4, 0
	.dword -8

	.word  ?MV, ?V, 0
	.word  32767, 0
	.word  32767, 0
	.dword 1073676289

	.word ?V, ?V, 0
	.word 0b1000000000000000, 0
	.word 0b1000000000000000, 0
	.word 0b0100000000000000, 0b0000000000000000
fin:

; XPCT int(sr) : 0
; XPCT oct(ir[10-15]) : 077

