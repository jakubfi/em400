; OPTS -c configs/multix.cfg

; Test winchester protocol:
;  - abort on non-configured multix
;  - abort on non-configured line
;  - abort on non-attached line
;  - abort on attached line
;  - abort on detached line

	.cpu	mx16

	.include hw.inc
	.include io.inc
	.include mx.inc

	uj	start

	.org	OS_MEM_BEG

msk_mx:	.word	IMASK_CH0_1

conf:	.word	1\7 | 3\15, 0
	.word	MX_LDIR_NONE | MX_LINE_USED | MX_LTYPE_WINCH | 3
	.word	MX_LPROTO_WINCH | 0, 3\7 | MX_SHORT_DISK_ADDR | MX_NO_FORMAT_PROTECT, 0, 0
	.word	MX_LPROTO_WINCH | 1, 3\7 | MX_SHORT_DISK_ADDR | MX_NO_FORMAT_PROTECT, 0, 0
	.word	MX_LPROTO_WINCH | 2, 3\7 | MX_SHORT_DISK_ADDR | MX_NO_FORMAT_PROTECT, 0, 0
mxdata:	.word	-1

; ------------------------------------------------------------------------

	.const	MXCMD_SETCFG	MX_IO_SETCFG | MX_CHAN
	.const	MXCMD_ABORT	MX_IO_ABORT | MX_CHAN
        .const	MXCMD_ATTACH	MX_IO_ATTACH | MX_CHAN
        .const	MXCMD_DETACH	MX_IO_DETACH | MX_CHAN

	.const	test_size	5 ; each test is: [io_type, command, arg, expected_irq, expected_result]
seq:
	; initial reset
	.word	0, 0, 0, MX_IWYZE, -1

	; abort on non-configured multix
	.word	c_in, MXCMD_ABORT | 0\10, mxdata, MX_INKAB + 0, -1

	; set configuration (OK)
	.word	c_ou, MXCMD_SETCFG, conf, MX_IUKON, -1

	; abort on non-configured line
	.word	c_in, MXCMD_ABORT | 3\10, mxdata, MX_INKAB + 3, -1

	; abort on non-attached line
	.word	c_in, MXCMD_ABORT | 0\10, mxdata, MX_INABT + 0, -1

	; abort on attached line
	.word	c_ou, MXCMD_ATTACH | 0\10, mxdata, MX_IDOLI + 0, -1
	.word	c_in, MXCMD_ABORT | 0\10, mxdata, MX_INABT + 0, -1

	; abort on detached line
	.word	c_in, MXCMD_DETACH | 0\10, mxdata, MX_IODLI + 0, -1
	.word	c_in, MXCMD_ABORT | 0\10, mxdata, MX_INABT + 0, -1
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
	rw	r1, MX_IV
	lw	r1, timer_proc
	rw	r1, IV_TIMER

	lw	r2, seq
	im	msk_mx

loop:	hlt
	ujs	loop

stack:

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT ir : 0xec3f
