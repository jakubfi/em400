	.include cpu.inc

	; exl should store IC, r0, SR and the argument on the stack, advance the stack pointer by 4,
	; jump to the EXL vector, zero r0, clear Q and the lowest RM bit, leave the rest of SR intact
	; exl should use block 0, not NB (unconfigured)
	lwt	r1, exlp
	rw	r1, EXLV
	lw	r1, stack
	rw	r1, STACKP
	im	rm
	mb	blk
	lw	r0, 0xfafa
	exl	0xa5
	hlt	040
exlp:	hlt	077

rm:	.word	0b1011010101000000
blk:	.word	0b1111111111011010

	.org	0x100
stack:

; XPCT r0 : 0
; XPCT sr : 0b1011010100011010
; XPCT [0x61] : 0x104
; XPCT [0x100] : 14
; XPCT [0x101] : 0xfafa
; XPCT [0x102] : 0b1011010101011010
; XPCT [0x103] : 0xa5
