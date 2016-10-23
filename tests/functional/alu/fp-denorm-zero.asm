; denormalized 0 doesn't set DIV0 int

	.include awp-fp.inc

operation:
	lf r7+ARG1
	af r7+ARG2
	uj r5

tests:
	.word 0, 0, 0
	.float 4
	.word 0x0000, 0x0000, 0x0001 ; 0 (denormalized)
	.float 4
fin:

; XPCT sr : 0
; XPCT ir&0x3f : 0o77
