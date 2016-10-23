
	.include awp-dword.inc

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

; XPCT sr : 0
; XPCT ir&0x3f : 0o77
