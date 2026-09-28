	.include cpu.inc

	; lip should restore IC, r0 and SR from the stack and pull the stack pointer back by 4
	lw	r1, stack
	rw	r1, STACKP
	lwt	r0, 0
	lip
	hlt	040

kim:	hlt	077

	.org	200
	.word	kim, 0xfafa, IMASK_ALL_MEM | 1, 0
stack:

; XPCT r0 : 0xfafa
; XPCT sr : 0xc001
; XPCT [0x61] : 200
