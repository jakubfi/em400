
	.include awp-fp.asm

operation:
	lf r7+ARG1
	af r7+ARG2
	uj r5

tests:
	.word 0, 0, INT_ENORM
	.word 0x2000, 0x0000, 0x0004 ; 4 (denormalized)
	.float 3
	.word 0x2000, 0x0000, 0x0004
fin:

; XPCT int(sr) : 0
; XPCT oct(ir[10-15]) : 077

