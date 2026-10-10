; [addr] without a segment reads block q*nb, which in user mode is the user block.
; A breakpoint stops the user program while the CPU still runs with q=1.

; PRECMD brk @2:0x102

	.include cpu.inc
	.include io.inc

	.const	USER_BLOCK	2
	.const	UADDR		0x100
	.const	DATA		0x200

	uj	start

e:	hlt	040

sys_sr:	.word	0
user_sr:.word	USER_BLOCK
userv:	.word	UADDR, 0, 1\SR_Q | USER_BLOCK

uprog:
	nop
	nop
	ujs	-1
uprog_end:

	.org	OS_START
start:
	lw	r1, 0\MEM_PAGE | USER_BLOCK\MEM_SEGMENT
	ou	r1, 3\MEM_FRAME | 0\MEM_MODULE | MEM_CFG
	.word	e, e, mapped, e
mapped:
	mb	user_sr
	lwt	r1, 0
copy:
	lw	r2, [r1+uprog]
	pw	r2, r1+UADDR
	awt	r1, 1
	cw	r1, uprog_end-uprog
	jn	copy
	lw	r1, 0x2222
	pw	r1, DATA
	mb	sys_sr
	sp	userv

	.org	DATA
	.word	0x1111

; POSTCMD brkdel 0

; XPCT q : 1
; XPCT nb : 2
; XPCT [0x200] : 0x2222
; XPCT [2:0x200] : 0x2222
; XPCT [0:0x200] : 0x1111
; XPCT_ERR [0x1000] : "Memory at 2:4096 is not configured (at 0-7)"
