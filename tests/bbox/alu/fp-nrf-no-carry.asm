
	.include awp-fp.asm

operation:
	lf r7+ARG1
	nrf
	uj r5

tests:
	.word 0, 0, 0
	.word 0x4000, 0x0000, 0x0100 ; 0.5000000.....00000001
	.float 0
	.word 0x4000, 0x0000, 0x0100 ; 0.5000000.....00000001
fin:

; XPCT int(sr) : 0
; XPCT oct(ir[10-15]) : 077

