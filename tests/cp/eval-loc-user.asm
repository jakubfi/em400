; @nb:addr matches the current instruction location in user mode.
; A breakpoint stops the user program, so the location is checked
; while the CPU still runs with q=1.

; PRECMD brk @2:0x102

	.include cpu.inc
	.include io.inc

	.const	USER_BLOCK	2
	.const	UADDR		0x100

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
	mb	sys_sr
	sp	userv

; XPCT q : 1
; XPCT nb : 2
; XPCT ic : 0x102
; XPCT @2:0x102 : 1
; XPCT @0:0x102 : 0
; XPCT @3:0x102 : 0
; XPCT @2:0x101 : 0
; XPCT @2:0x103 : 0
; XPCT !@2:0x102 : 0
; XPCT @2:0x102 && q : 1
; XPCT @2:0x102 + @2:0x102 : 2

; POSTCMD brkdel 0
