; OPTS -c configs/multix.cfg

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

	.include hw.inc
	.include io.inc
	.include mx.inc

	UJ	start

	.org	OS_MEM_BEG

msk_mx:	.word	IMASK_CH0_1

conf:	.word	1\7 | 4\15, 0
	.word	MX_LDIR_NONE | MX_LINE_USED | MX_LTYPE_WINCH | 3
	.word	MX_LPROTO_WINCH | 0, 3\7 | MX_SHORT_DISK_ADDR | MX_NO_FORMAT_PROTECT, 0, 0
	.word	MX_LPROTO_WINCH | 1, 3\7 | MX_SHORT_DISK_ADDR | MX_NO_FORMAT_PROTECT, 0, 0
	.word	MX_LPROTO_WINCH | 2, 3\7 | MX_SHORT_DISK_ADDR | MX_NO_FORMAT_PROTECT, 0, 0
	.word	MX_LPROTO_WINCH | 3, 3\7 | MX_SHORT_DISK_ADDR | MX_NO_FORMAT_PROTECT, 0, 0
mxdata:	.word	-1

; ------------------------------------------------------------------------

	.const	MXCMD_SETCFG	MX_IO_SETCFG | MX_CHAN_DEFAULT
	.const	MXCMD_STATUS	MX_IO_STATUS | MX_CHAN_DEFAULT
	.const	MXCMD_ATTACH	MX_IO_ATTACH | MX_CHAN_DEFAULT
	.const	MXCMD_DETACH	MX_IO_DETACH | MX_CHAN_DEFAULT

	.const	test_size	5 ; each test is: [io_type, command, arg, expected_irq, expected_result]
seq:
	; initial reset
	.word	0,    0,                   0,      MX_IWYZE\7, -1

	; set configuration (OK)
	.word	c_ou, MXCMD_SETCFG,        conf,   MX_IUKON\7, -1

	; check line status (OK)
	.word	c_ou, MXCMD_STATUS | 0\10, mxdata, MX_ISTRE\7 + 0, MX_LSTATE_NONE
	.word	c_ou, MXCMD_STATUS | 1\10, mxdata, MX_ISTRE\7 + 1, MX_LSTATE_NONE
	.word	c_ou, MXCMD_STATUS | 2\10, mxdata, MX_ISTRE\7 + 2, MX_LSTATE_NONE
	.word	c_ou, MXCMD_STATUS | 3\10, mxdata, MX_ISTRE\7 + 3, MX_LSTATE_NONE

	; check satus for nonexistent line (error)
	.word	c_ou, MXCMD_STATUS | 4\10, mxdata, MX_INKST\7 + 4, MX_LSTATE_NONE

	; attach lines (OK)
	.word	c_ou, MXCMD_ATTACH | 0\10, mxdata, MX_IDOLI\7 + 0, -1
	.word	c_ou, MXCMD_ATTACH | 1\10, -1, MX_IDOLI\7 + 1, -1
	.word	c_ou, MXCMD_ATTACH | 2\10, mxdata, MX_IDOLI\7 + 2, -1
	.word	c_ou, MXCMD_ATTACH | 3\10, mxdata, MX_IDOLI\7 + 3, -1

	; check line status (OK)
	.word	c_ou, MXCMD_STATUS | 0\10, mxdata, MX_ISTRE\7 + 0, MX_LSTATE_ATTACHED
	.word	c_ou, MXCMD_STATUS | 1\10, mxdata, MX_ISTRE\7 + 1, MX_LSTATE_ATTACHED
	.word	c_ou, MXCMD_STATUS | 2\10, mxdata, MX_ISTRE\7 + 2, MX_LSTATE_ATTACHED
	.word	c_ou, MXCMD_STATUS | 3\10, mxdata, MX_ISTRE\7 + 3, MX_LSTATE_ATTACHED

	; check line status with bogus result address (error)
	.word	c_ou, MXCMD_STATUS | 3\10, -1, MX_INPAO\7 + 3, -1

	; attach nonexistent line (error)
	.word	c_ou, MXCMD_ATTACH | 4\10, mxdata, MX_INKDO\7 + 4, -1

	; attach already attached lines (error)
	.word	c_ou, MXCMD_ATTACH | 0\10, mxdata, MX_INDOL\7 + 0, -1
	.word	c_ou, MXCMD_ATTACH | 1\10, mxdata, MX_INDOL\7 + 1, -1
	.word	c_ou, MXCMD_ATTACH | 2\10, -1, MX_INDOL\7 + 2, -1
	.word	c_ou, MXCMD_ATTACH | 3\10, mxdata, MX_INDOL\7 + 3, -1

	; detach lines (OK)
	.word	c_in, MXCMD_DETACH | 0\10, mxdata, MX_IODLI\7 + 0, -1
	.word	c_in, MXCMD_DETACH | 1\10, mxdata, MX_IODLI\7 + 1, -1
	.word	c_in, MXCMD_DETACH | 2\10, mxdata, MX_IODLI\7 + 2, -1
	.word	c_in, MXCMD_DETACH | 3\10, -1, MX_IODLI\7 + 3, -1

	; check line status (OK)
	.word	c_ou, MXCMD_STATUS | 0\10, mxdata, MX_ISTRE\7 + 0, MX_LSTATE_NONE
	.word	c_ou, MXCMD_STATUS | 1\10, mxdata, MX_ISTRE\7 + 1, MX_LSTATE_NONE
	.word	c_ou, MXCMD_STATUS | 2\10, mxdata, MX_ISTRE\7 + 2, MX_LSTATE_NONE
	.word	c_ou, MXCMD_STATUS | 3\10, mxdata, MX_ISTRE\7 + 3, MX_LSTATE_NONE

	; detach already detached lines (also OK)
	.word	c_in, MXCMD_DETACH | 0\10, -1, MX_IODLI\7 + 0, -1
	.word	c_in, MXCMD_DETACH | 1\10, mxdata, MX_IODLI\7 + 1, -1
	.word	c_in, MXCMD_DETACH | 2\10, mxdata, MX_IODLI\7 + 2, -1
	.word	c_in, MXCMD_DETACH | 3\10, mxdata, MX_IODLI\7 + 3, -1

	; check line status (OK)
	.word	c_ou, MXCMD_STATUS | 0\10, mxdata, MX_ISTRE\7 + 0, MX_LSTATE_NONE
	.word	c_ou, MXCMD_STATUS | 1\10, mxdata, MX_ISTRE\7 + 1, MX_LSTATE_NONE
	.word	c_ou, MXCMD_STATUS | 2\10, mxdata, MX_ISTRE\7 + 2, MX_LSTATE_NONE
	.word	c_ou, MXCMD_STATUS | 3\10, mxdata, MX_ISTRE\7 + 3, MX_LSTATE_NONE

seqe:

; ------------------------------------------------------------------------
; expects:
;  r5 - I/O command type (OU/IN)
;  r3 - I/O command
;  r1 - configuration field address
;  r4 - RJ return adress
io_cmd:
	UJ	r5
c_ou:	OU	r1, r3
	.word	c_no, c_en, c_ok, c_pe
c_in:	IN	r1, r3
	.word	c_no, c_en, c_ok, c_pe
c_no:	HLT	041	; error
c_ok:	UJ	r4	; return
c_en:	UJ	r5	; repeat if engaged
c_pe:	HLT	042	; error

; ------------------------------------------------------------------------
mx_proc:

	LW	r1, [STACKP]
	LW	r1, [r1-1]	; get intspec from multix

	; check intspec
	CW	r1, [r2+3]
	BB	r0, ?E
	HLT	040

	; should we check result?
	LW	r3, [r2+4]
	CWT	r3, -1
	JES	no_res_check

	; check result
	LW	r5, [r2+2]
	CW	r3, [r5]
	BB	r0, ?E
	HLT	043

no_res_check:
	; load and run next test
	AWT	r2, test_size
	CW	r2, seqe	; all tests finished?
	BLC	?E
	HLT	077

	LW	r5, [r2]	; load io type
	LW	r3, [r2+1]	; load next command
	LW	r1, [r2+2]	; load next argument
	RJ	r4, io_cmd

	LIP

; ------------------------------------------------------------------------
timer_proc:
	LIP

; ------------------------------------------------------------------------
start:
	LW	r1, stack
	RW	r1, STACKP
	LW	r1, mx_proc
	RW	r1, MX_IV
	LW	r1, timer_proc
	RW	r1, IV_TIMER

	LW	r2, seq
	IM	msk_mx

loop:	HLT
	UJS	loop

stack:

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT ir : 0xec3f
