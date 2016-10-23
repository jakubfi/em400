
	.include awp-fp.inc

operation:
	lf r7+ARG1
	af r7+ARG2
	uj r5

tests:
	.word 0, 0, 0
	.float 0.5
	.word 0x4000, 0x0000, 0x0200 ; 0.5000000.....000000010
	.word 0x4000, 0x0000, 0x0101
fin:

; XPCT sr : 0
; XPCT ir&0x3f : 0o77
