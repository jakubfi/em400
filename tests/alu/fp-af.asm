
	.include awp-fp.inc

operation:
	lf r7+ARG1
	af r7+ARG2
	uj r5

tests:
	.word 0, 0, 0
	.float 1
	.float 1000000000
	.float 1000000001

	.word 0, ?M, 0
	.float -100
	.float 1
	.float -99

	.word 0, 0, 0
	.float 1000000000
	.float 0.125
	.float 1000000000.125
fin:

; XPCT int(sr) : 0
; XPCT oct(ir[10-15]) : 077

