
	.include awp-fp.inc

operation:
	lf r7+ARG1
	nrf
	uj r5

tests:
	.word 0, ?M, 0
	.float -0.1
	.float 0
	.float -0.1
fin:

; XPCT int(sr) : 0
; XPCT oct(ir[10-15]) : 077

