
	.include awp-fp.asm

operation:
	lf r7+ARG1
	df r7+ARG2
	uj r5

tests:
	.word 0, ?M, 0
	.float 4
	.word 0x9999, 0x9999, 0x9afd ; a bit less than -0.1
	.word 0xafff, 0xffff, 0xff06 ; a bit less than -40
fin:

; XPCT int(sr) : 0
; XPCT oct(ir[10-15]) : 077

