
	.include awp-fp.inc

operation:
	lf	r7+ARG1
	df	r7+ARG2
	uj	r5

tests:
	.word	0, ?M, 0
	.float	4
	.word	0x9999, 0x9999, 0x9afd ; a bit less than -0.1
	.word	0xb000, 0x0000, 0x0006
fin:

; XPCT sr : 0
; XPCT ir : 0xec3f
