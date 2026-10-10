; [addr] without a segment reads block q*nb, so nb is ignored in system mode.

	.include cpu.inc
	.include io.inc

	.const	USER_BLOCK	2
	.const	DATA		0x200

	uj	start

e:	hlt	040

user_sr:.word	USER_BLOCK

	.org	OS_START
start:
	lw	r1, 0\MEM_PAGE | USER_BLOCK\MEM_SEGMENT
	ou	r1, 3\MEM_FRAME | 0\MEM_MODULE | MEM_CFG
	.word	e, e, mapped, e
mapped:
	mb	user_sr
	lw	r1, 0x2222
	pw	r1, DATA
	hlt	077

	.org	DATA
	.word	0x1111

; XPCT q : 0
; XPCT nb : 2
; XPCT [0x200] : 0x1111
; XPCT [0:0x200] : 0x1111
; XPCT [2:0x200] : 0x2222
