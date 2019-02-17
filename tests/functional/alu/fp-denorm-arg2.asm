
	.include awp-fp.inc

operation:
	lf	r7+ARG1
	af	r7+ARG2
	uj	r5

tests:
	.word	0, 0, I_FP_ERR
	.float	3
	.word	0x2000, 0x0000, 0x0004 ; 4 (denormalized)
	.float	3
fin:

; XPCT sr : 0
; XPCT ir : 0xec3f
