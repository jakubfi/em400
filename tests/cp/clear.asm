; CLEAR resets r0, SR, mc, interrupts and memory configuration,
; but leaves user registers and IC alone.

	.include cpu.inc
	.include io.inc

; PRECMD brk mc==1

	uj	start
err:	hlt	040
mask:	.word	IMASK_CH0_1

	.org	OS_START
start:
	lw	r1, 1\MEM_SEGMENT | 2\MEM_PAGE
	ou	r1, 0\MEM_MODULE | 2\MEM_FRAME | MEM_CFG
	.word	err, err, ok, err
ok:
	im	mask
	lw	r0, 0xff00
	lw	r1, 0x1234
	md	1
	hlt	077

; XPCT_CMD int 10 : "OK: 0x00200000"
; XPCT_CMD memmap 1 : "OK: 0x0004"
; XPCT_CMD reg r0 : "OK: 0xff00"
; XPCT_CMD reg sr : "OK: 0x0400"
; XPCT_CMD eval mc : "OK: 1"
; XPCT_CMD reg ic : "OK: 0x0080"

; XPCT_CMD clear : "OK: CLEAR"
; XPCT_CMD int : "OK: 0x00000000"
; XPCT_CMD memmap 1 : "OK: 0x0000"
; XPCT_CMD reg r0 : "OK: 0x0000"
; XPCT_CMD reg sr : "OK: 0x0000"
; XPCT_CMD eval mc : "OK: 0"
; XPCT_CMD reg r1 : "OK: 0x1234"
; XPCT_CMD reg ic : "OK: 0x0080"

; POSTCMD brkdel 0
