
	.include awp-fp.asm

operation:
	lf r7+ARG1
	df r7+ARG2
	uj r5

tests:
	.word 0, 0, INT_DIV0
	.float 5
	.float 0
	.float 5
fin:

; XPCT int(sr) : 0
; XPCT oct(ir[10-15]) : 077

