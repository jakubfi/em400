
	.include awp-fp.inc

operation:
	lf r7+ARG1
	af r7+ARG2
	uj r5

tests:
	.word 0, ?M, 0
	.float 2
	.float -3
	.float -1
fin:

; XPCT sr : 0
; XPCT ir&0x3f : 0o77
