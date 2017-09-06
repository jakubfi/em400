
	.include awp-fp.inc

operation:
	lf	r7+ARG1
	nrf
	uj	r5

tests:
	.word	0, 0, 0
	.word	0x2000, 0x0000, 0x0004 ; 4 (denormalized)
	.float	0
	.float	4
fin:

; XPCT sr : 0
; XPCT ir : 0xec3f
