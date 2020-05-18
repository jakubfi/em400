; OPTS -c configs/multix.ini

; Test winchester protocol:
;  - get initial line status
;  - get status for nonexistent line
;  - get status for attached lines
;  - get status with bogus resuld field address (-1)
;
;  - attach lines
;  - attach nonexistent line
;  - attach already attached line
;  - detach line
;  - detach already detached line

	.cpu	mx16

	.include cpu.inc
	.include io.inc
	.include multix.inc

	uj	start

	.org	OS_START

msk_mx:	.word	IMASK_CH0_1

conf:	.word	1\7 | 4\15, 0
	.word	MX_LDIR_NONE | MX_LINE_USED | MX_LTYPE_WINCH | 3
	.word	MX_LPROTO_WINCH | 0, 3\7 | MX_SHORT_DISK_ADDR | MX_NO_FORMAT_PROTECT, 0, 0
	.word	MX_LPROTO_WINCH | 1, 3\7 | MX_SHORT_DISK_ADDR | MX_NO_FORMAT_PROTECT, 0, 0
	.word	MX_LPROTO_WINCH | 2, 3\7 | MX_SHORT_DISK_ADDR | MX_NO_FORMAT_PROTECT, 0, 0
	.word	MX_LPROTO_WINCH | 3, 3\7 | MX_SHORT_DISK_ADDR | MX_NO_FORMAT_PROTECT, 0, 0
mxdata:	.word	-1

; ------------------------------------------------------------------------

	.const	MXCMD_SETCFG	MX_CMD_SETCFG | 1\IO_CHAN
	.const	MXCMD_STATUS	MX_CMD_STATUS | 1\IO_CHAN
	.const	MXCMD_ATTACH	MX_CMD_ATTACH | 1\IO_CHAN
	.const	MXCMD_DETACH	MX_CMD_DETACH | 1\IO_CHAN

	.const	test_size	5 ; each test is: [io_type, command, arg, expected_irq, expected_result]
seq:
	; initial reset
	.word	0, 0, 0, MX_IWYZE, -1

	; set configuration (OK)
	.word	c_ou, MXCMD_SETCFG, conf, MX_IUKON, -1

	; check line status (OK)
	.word	c_ou, MXCMD_STATUS | 0\10, mxdata, MX_ISTRE + 0, MX_LSTATE_NONE
	.word	c_ou, MXCMD_STATUS | 1\10, mxdata, MX_ISTRE + 1, MX_LSTATE_NONE
	.word	c_ou, MXCMD_STATUS | 2\10, mxdata, MX_ISTRE + 2, MX_LSTATE_NONE
	.word	c_ou, MXCMD_STATUS | 3\10, mxdata, MX_ISTRE + 3, MX_LSTATE_NONE

	; check satus for nonexistent line (error)
	.word	c_ou, MXCMD_STATUS | 4\10, mxdata, MX_INKST + 4, MX_LSTATE_NONE

	; attach lines (OK)
	.word	c_ou, MXCMD_ATTACH | 0\10, mxdata, MX_IDOLI + 0, -1
	.word	c_ou, MXCMD_ATTACH | 1\10, -1, MX_IDOLI + 1, -1
	.word	c_ou, MXCMD_ATTACH | 2\10, mxdata, MX_IDOLI + 2, -1
	.word	c_ou, MXCMD_ATTACH | 3\10, mxdata, MX_IDOLI + 3, -1

	; check line status (OK)
	.word	c_ou, MXCMD_STATUS | 0\10, mxdata, MX_ISTRE + 0, MX_LSTATE_ATTACHED
	.word	c_ou, MXCMD_STATUS | 1\10, mxdata, MX_ISTRE + 1, MX_LSTATE_ATTACHED
	.word	c_ou, MXCMD_STATUS | 2\10, mxdata, MX_ISTRE + 2, MX_LSTATE_ATTACHED
	.word	c_ou, MXCMD_STATUS | 3\10, mxdata, MX_ISTRE + 3, MX_LSTATE_ATTACHED

	; check line status with bogus result address (error)
	.word	c_ou, MXCMD_STATUS | 3\10, -1, MX_INPAO + 3, -1

	; attach nonexistent line (error)
	.word	c_ou, MXCMD_ATTACH | 4\10, mxdata, MX_INKDO + 4, -1

	; attach already attached lines (error)
	.word	c_ou, MXCMD_ATTACH | 0\10, mxdata, MX_INDOL + 0, -1
	.word	c_ou, MXCMD_ATTACH | 1\10, mxdata, MX_INDOL + 1, -1
	.word	c_ou, MXCMD_ATTACH | 2\10, -1, MX_INDOL + 2, -1
	.word	c_ou, MXCMD_ATTACH | 3\10, mxdata, MX_INDOL + 3, -1

	; detach lines (OK)
	.word	c_in, MXCMD_DETACH | 0\10, mxdata, MX_IODLI + 0, -1
	.word	c_in, MXCMD_DETACH | 1\10, mxdata, MX_IODLI + 1, -1
	.word	c_in, MXCMD_DETACH | 2\10, mxdata, MX_IODLI + 2, -1
	.word	c_in, MXCMD_DETACH | 3\10, -1, MX_IODLI + 3, -1

	; check line status (OK)
	.word	c_ou, MXCMD_STATUS | 0\10, mxdata, MX_ISTRE + 0, MX_LSTATE_NONE
	.word	c_ou, MXCMD_STATUS | 1\10, mxdata, MX_ISTRE + 1, MX_LSTATE_NONE
	.word	c_ou, MXCMD_STATUS | 2\10, mxdata, MX_ISTRE + 2, MX_LSTATE_NONE
	.word	c_ou, MXCMD_STATUS | 3\10, mxdata, MX_ISTRE + 3, MX_LSTATE_NONE

	; detach already detached lines (also OK)
	.word	c_in, MXCMD_DETACH | 0\10, -1, MX_IODLI + 0, -1
	.word	c_in, MXCMD_DETACH | 1\10, mxdata, MX_IODLI + 1, -1
	.word	c_in, MXCMD_DETACH | 2\10, mxdata, MX_IODLI + 2, -1
	.word	c_in, MXCMD_DETACH | 3\10, mxdata, MX_IODLI + 3, -1

	; check line status (OK)
	.word	c_ou, MXCMD_STATUS | 0\10, mxdata, MX_ISTRE + 0, MX_LSTATE_NONE
	.word	c_ou, MXCMD_STATUS | 1\10, mxdata, MX_ISTRE + 1, MX_LSTATE_NONE
	.word	c_ou, MXCMD_STATUS | 2\10, mxdata, MX_ISTRE + 2, MX_LSTATE_NONE
	.word	c_ou, MXCMD_STATUS | 3\10, mxdata, MX_ISTRE + 3, MX_LSTATE_NONE

seqe:

; ------------------------------------------------------------------------
; expects:
;  r5 - I/O command type (OU/IN)
;  r3 - I/O command
;  r1 - configuration field address
;  r4 - RJ return adress
io_cmd:
	uj	r5
c_ou:	ou	r1, r3
	.word	c_no, c_en, c_ok, c_pe
c_in:	in	r1, r3
	.word	c_no, c_en, c_ok, c_pe
c_no:	hlt	041	; error
c_ok:	uj	r4	; return
c_en:	uj	r5	; repeat if engaged
c_pe:	hlt	042	; error

; ------------------------------------------------------------------------
mx_proc:

	lw	r1, [STACKP]
	lw	r1, [r1-1]	; get intspec from multix

	; check intspec
	cw	r1, [r2+3]
	bb	r0, ?E
	hlt	040

	; should we check result?
	lw	r3, [r2+4]
	cwt	r3, -1
	jes	no_res_check

	; check result
	lw	r5, [r2+2]
	cw	r3, [r5]
	bb	r0, ?E
	hlt	043

no_res_check:
	; load and run next test
	awt	r2, test_size
	cw	r2, seqe	; all tests finished?
	blc	?E
	hlt	077

	lw	r5, [r2]	; load io type
	lw	r3, [r2+1]	; load next command
	lw	r1, [r2+2]	; load next argument
	rj	r4, io_cmd

	lip

; ------------------------------------------------------------------------
timer_proc:
	lip

; ------------------------------------------------------------------------
start:
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, mx_proc
	rw	r1, INTV_CH1
	lw	r1, timer_proc
	rw	r1, INTV_TIMER

	lw	r2, seq
	im	msk_mx

loop:	hlt
	ujs	loop

stack:

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT ir : 0xec3f
