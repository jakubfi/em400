
	.include awp-fp.inc

operation:
	lf r7+ARG1
	af r7+ARG2
	uj r5

tests:
	.word 0, ?Z, 0
	.float 1
	.float -1
	.float 0
fin:

; XPCT sr : 0
; XPCT ir : 0xec3f
