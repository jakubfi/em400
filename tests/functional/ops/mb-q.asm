	.include cpu.inc

	uj	start

exl_handler:
	hlt	077

rm:	.word	0b0101010101000000
blk:	.word	0b1111111111100000

	; mb with Q=1 should switch to user mode, SR is checked in the frame saved by exl
start:
	lwt	r1, exl_handler
	rw	r1, EXLV
	lw	r1, stack
	rw	r1, STACKP
	im	rm
	mb	blk
	exl	0

	.org	0x100
stack:

; XPCT [0x102] : 0b0101010101100000
