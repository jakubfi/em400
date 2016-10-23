
	.include awp-fp.inc

operation:
	lf r7+ARG1
	mf r7+ARG2
	uj r5

tests:
	.word 0, 0, 0
	.float 2
	.float 2
	.float 4

	.word 0, ?M, 0
	.float 1000000000
	.float -100
	.float -100000000000.0

	.word 0, 0, 0
	.float 2
	.float 0.125
	.float 0.25
fin:

; XPCT sr : 0
; XPCT ir&0x3f : 0o77
