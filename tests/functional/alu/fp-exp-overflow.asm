
	.include awp-fp.inc

operation:
	lf r7+ARG1
	mf r7+ARG2
	uj r5

tests:
	.word 0, 0, INT_FP_OF
	.word 0x7fff, 0xffff, 0xff7f ; 0.9999... * 2^127
	.float 2
	.word 0x7fff, 0xffff, 0xff80 ; garbage
fin:

; XPCT sr : 0
; XPCT ir&0x3f : 0o77
