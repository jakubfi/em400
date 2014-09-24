
	.include awp-dword.inc

operation:
	ld r7+ARG1
	dw r7+ARG2
	uj r5

tests:
	.word 0, 0, INT_DIV_OF
	.word 0b0111111111111111, 0b1111111111111111
	.word 2, 0
	.word 0b0111111111111111, 0b1111111111111111
fin:

; XPCT int(sr) : 0
; XPCT oct(ir[10-15]) : 077

