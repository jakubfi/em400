
	.include awp-fp.inc

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

; XPCT sr : 0
; XPCT ir&0x3f : 0o77
