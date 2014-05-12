
	.include awp-dword.asm

operation:
	ld r7+ARG1
	dw r7+ARG2
	uj r5

tests:
	.word  0, 0, INT_DIV0
	.dword 5
	.word  0, 0
	.dword 5
fin:

; XPCT int(sr) : 0
; XPCT oct(ir[10-15]) : 077

