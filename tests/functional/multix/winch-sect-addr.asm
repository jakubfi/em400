; OPTS -c configs/winchester0.ini
; PRECMD CLOCK OFF

	.cpu	mx16

	.include cpu.inc
	.include io.inc
	.include multix.inc

	; use bit 15 of the register I/O argument for IN/OU selection
	.const IO_IN 1
	.const IO_OU 0

	; winchester device address
	.const	WINCH_LINE 2
	.const	WINCH_ADDR 1\IO_CHAN | WINCH_LINE\10

	; MULTIX command shortcuts
	.const	MXCMD_SETCFG	MX_CMD_SETCFG | 1\IO_CHAN | IO_OU
	.const	MXCMD_STATUS	MX_CMD_STATUS | WINCH_ADDR | IO_OU
	.const	MXCMD_ATTACH	MX_CMD_ATTACH | WINCH_ADDR | IO_OU
	.const	MXCMD_DETACH	MX_CMD_DETACH | WINCH_ADDR | IO_IN
	.const	MXCMD_TRANSMIT	MX_CMD_TRANSMIT | WINCH_ADDR | IO_OU
	.const	MXCMD_ABORT	MX_CMD_ABORT | WINCH_ADDR | IO_IN

	; Winchester geometry
	.const	CYLINDERS 614 ; 1 cylinder reserved by multix for spare sectors
	.const	HEADS 4
	.const	SPT 16
	.const	SSIZE 256
	.const	USER_SECTORS CYLINDERS*HEADS*SPT

	mcl
	uj	start

msk_0:	.word	IMASK_NONE
msk_mx:	.word	IMASK_CH0_1
xlip:	lip

	.org	INTV
	.res	16, xlip	; dummy interrupt handlers
	.word	xlip		; dummy EXL handler

	.org	OS_START

; ------------------------------------------------------------------------
; test data
conf:	.word	1\7 | 3\15, 0
	.word	MX_LDIR_NONE | MX_LINE_USED | MX_LTYPE_WINCH | 3
	.word	MX_LPROTO_WINCH | 2, 3\7 | MX_SHORT_DISK_ADDR | MX_NO_FORMAT_PROTECT, 0, 0
	.word	MX_LPROTO_WINCH | 1, 3\7 | MX_SHORT_DISK_ADDR | MX_NO_FORMAT_PROTECT, 0, 0
	.word	MX_LPROTO_WINCH | 0, 3\7 | MX_SHORT_DISK_ADDR | MX_NO_FORMAT_PROTECT, 0, 0
readf:	.word	2\7 + 0\15, rdbuf, 255, 0, USER_SECTORS-1, -1, -1
	.const	SECT_POS 4 ; position of sector address
	; test vector: [loops, command, field_addr, exp_irq, check_proc]
	.const	LOOPS 0
	.const	CMD LOOPS+1
	.const	FIELD CMD+1
	.const	INT FIELD+1
	.const	PROC INT+1
	.const	TLEN PROC+1
seq:
	.word	0, -1, -1, MX_IWYZE, 0
	.word	0, MXCMD_SETCFG, conf, MX_IUKON, 0
	.word	0, MXCMD_ATTACH, -1, MX_IDOLI + WINCH_LINE, 0
	.word   USER_SECTORS-1, MXCMD_TRANSMIT, readf, MX_IETRA + WINCH_LINE, addr_check
	.word	0, MXCMD_DETACH, -1, MX_IODLI + WINCH_LINE, 0
seqe:

; ------------------------------------------------------------------------
addr_check:
	.res	1
	; check if sector address is correct
	lw	r2, [rdbuf]
	cw	r2, [r7+LOOPS]
	bb	r0, ?E
	hlt	045
	; decrement sector number
	awt	r2, -1
	rw	r2, readf+SECT_POS
	uj	[addr_check]

; ------------------------------------------------------------------------
mx_proc:
	; get and check int spec
	md	[STACKP]
	lw	r1, [-1]
	cw	r1, [r7+INT]
	bb	r0, ?E
	hlt	044

	; check test result
	lw	r1, [r7+PROC]
	cw	r1, 0
	bb	r0, ?E
	lj	addr_check

	; advance test
	lw	r1, [r7+LOOPS]
	awt	r1, -1
	rw	r1, r7+LOOPS
	cw	r1, -1
	jn	test_cont
	awt	r7, TLEN
	cw	r7, seqe
	bc	r0, ?E
	hlt	077

	; send next I/O
test_cont:
	lw	r1, [r7+CMD]
	lw	r2, [r7+FIELD]
	sxl	r1
	er	r1, 1
repeat:	jxs	c_in
c_ou:	ou	r2, r1
	.word	c_no, c_en, c_ok, c_pe
c_in:	in	r2, r1
	.word	c_no, c_en, c_ok, c_pe
c_no:	hlt	041	; error
c_en:	ujs	repeat	; repeat if engaged
c_pe:	hlt	042	; error
c_ok:
	lip

; ------------------------------------------------------------------------
; ---- MAIN --------------------------------------------------------------
; ------------------------------------------------------------------------
; r7 - current test
start:
	; initialize interrupt system
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, mx_proc
	rw	r1, INTV_CH1

	; initialize test sequence
	lw	r7, seq

	; enable interrupts, enter idle loop
	im	msk_mx
idle:	hlt
	ujs	idle

prog_end:
	.const	rdbuf prog_end
	.const	stack rdbuf+SSIZE

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT ir : 0xec3f
