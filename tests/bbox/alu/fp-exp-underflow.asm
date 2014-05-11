
	.include awp-fp.asm

operation:
	lf r7+ARG1
	df r7+ARG2
	uj r5

tests:
	.word 0, 0, INT_FP_UF
	.word 0x7fff, 0xffff, 0xff80 ; 0.9999... * 2^-128
	.float 2
	.word 0x7fff, 0xffff, 0xff7f ; garbage
fin:

; XPCT int(sr) : 0
; XPCT oct(ir[10-15]) : 077

