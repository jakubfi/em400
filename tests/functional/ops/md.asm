	.include cpu.inc

	uj	start

exl_handler:
	hlt	077

user_sr:.word	0b0000000000100000

start:
	lwt	r1, exl_handler
	rw	r1, EXLV
	lw	r1, stack
	rw	r1, STACKP

	; md should modify the argument of the next instruction
	md	1
	lwt	r1, 1

	; md should be legal in user mode
	mb	user_sr
	md	1
	lwt	r2, 1
	exl	0

	.org	0x100
stack:

; XPCT r1 : 2
; XPCT r2 : 2
