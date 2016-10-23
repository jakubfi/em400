
	.include awp-fp.inc

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

; XPCT sr : 0
; XPCT ir&0x3f : 0o77
