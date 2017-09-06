
	.include awp-fp.inc

operation:
	lf	r7+ARG1
	nrf
	uj	r5

tests:
	.word	0, ?M, 0
	.float	-0.1
	.float	0
	.float	-0.1
fin:

; XPCT sr : 0
; XPCT ir : 0xec3f
