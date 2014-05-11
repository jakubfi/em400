
	.include awp-fp.asm

operation:
	lf r7+ARG1
	sf r7+ARG2
	uj r5

tests:
	.word 0, ?M, 0
	.float -100
	.float 0.125
	.float -100.125

	.word 0, ?M, 0
	.float 0.125
	.float 1000000000
	.float -999999999.875

	.word 0, 0, 0
	.float 1000000000
	.float 1
	.float 999999999
fin:

; XPCT int(sr) : 0
; XPCT oct(ir[10-15]) : 077

