
	.include awp-fp.asm

operation:
	lf r7+ARG1
	af r7+ARG2
	uj r5

tests:
	.word 0, ?Z, 0
	.float 1
	.float -1
	.float 0
fin:

; XPCT int(sr) : 0
; XPCT oct(ir[10-15]) : 077

