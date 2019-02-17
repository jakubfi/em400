
	.include awp-fp.inc

operation:
	lf	r7+ARG1
	df	r7+ARG2
	uj	r5

tests:
	.word	0, 0, I_FP_UF
	.word	0x7fff, 0xffff, 0xff80 ; 0.9999... * 2^-128
	.float	2
	.word	0x7fff, 0xffff, 0xff7f ; garbage
fin:

; XPCT sr : 0
; XPCT ir : 0xec3f
