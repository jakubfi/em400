; OPTS -c configs/multix.cfg

	.cpu	mx16

	.include cpu.inc
	.include io.inc
	.include multix.inc

	mcl
	uj	start

msk_mx:	.word	IMASK_CH0_1

	.org	OS_START

; ---------------------------------------------------------------
; logical line configuration tests
; ---------------------------------------------------------------

dummy:	.word	0, 0 ; dummy to acknowledge IWYZE

; wrong directions for punch reader (SOM)

dpr0s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_USARTA + 3
	.word	MX_LPROTO_SOM_PUNCHRD + 2\15, 0, 0, 0 ; wrong direction (out) for SOM punch reader on line 0
dpr1s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_8255 + 3
	.word	MX_LPROTO_SOM_PUNCHRD + 2\15, 0, 0, 0 ; wrong direction (out) for SOM punch reader on line 0
dpr2s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_SOM_PUNCHRD + 2\15, 0, 0, 0 ; wrong direction (out) for SOM punch reader on line 0
dpr3s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_WINCH + 3
	.word	MX_LPROTO_SOM_PUNCHRD + 2\15, 0, 0, 0 ; wrong direction (none) for SOM punch reader on line 0
dpr4s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_TAPE + 3
	.word	MX_LPROTO_SOM_PUNCHRD + 2\15, 0, 0, 0 ; wrong direction (none) for SOM punch reader on line 0
dpr5s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_FLOPPY + 3
	.word	MX_LPROTO_SOM_PUNCHRD + 2\15, 0, 0, 0 ; wrong direction (none) for SOM punch reader on line 0

; wrong directions for puncher (SOM)

dpu0s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTA + 3
	.word	MX_LPROTO_SOM_PUNCHER + 2\15, 0, 0, 0 ; wrong direction (in) for SOM puncher on line 0
dpu1s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_8255 + 3
	.word	MX_LPROTO_SOM_PUNCHER + 2\15, 0, 0, 0 ; wrong direction (in) for SOM puncher on line 0
dpu2s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_SOM_PUNCHER + 2\15, 0, 0, 0 ; wrong direction (in) for SOM puncher on line 0
dpu3s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_WINCH + 3
	.word	MX_LPROTO_SOM_PUNCHER + 2\15, 0, 0, 0 ; wrong direction (none) for SOM puncher on line 0
dpu4s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_TAPE + 3
	.word	MX_LPROTO_SOM_PUNCHER + 2\15, 0, 0, 0 ; wrong direction (none) for SOM puncher on line 0
dpu5s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_FLOPPY + 3
	.word	MX_LPROTO_SOM_PUNCHER + 2\15, 0, 0, 0 ; wrong direction (none) for SOM puncher on line 0

; wrong directions for monitor (SOM)

dm0s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTA + 3
	.word	MX_LPROTO_SOM_MONITOR + 2\15, 0, 0, 0 ; wrong direction (in) for SOM monitor on line 0
dm1s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_USARTA + 3
	.word	MX_LPROTO_SOM_MONITOR + 2\15, 0, 0, 0 ; wrong direction (out) for SOM monitor on line 0
dm2s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_WINCH + 3
	.word	MX_LPROTO_SOM_MONITOR + 2\15, 0, 0, 0 ; wrong direction (none) for SOM monitor on line 0
dm3s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_TAPE + 3
	.word	MX_LPROTO_SOM_MONITOR + 2\15, 0, 0, 0 ; wrong direction (none) for SOM monitor on line 0
dm4s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_FLOPPY + 3
	.word	MX_LPROTO_SOM_MONITOR + 2\15, 0, 0, 0 ; wrong direction (none) for SOM monitor on line 0

dm5s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_SOM_MONITOR + 2\15, 0, 0, 0 ; wrong direction (none) for SOM monitor on line 0
dm6s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_SOM_MONITOR + 2\15, 0, 0, 0 ; wrong direction (none) for SOM monitor on line 0
dm7s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_8255 + 3
	.word	MX_LPROTO_SOM_MONITOR + 2\15, 0, 0, 0 ; wrong direction (none) for SOM monitor on line 0
dm8s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_8255 + 3
	.word	MX_LPROTO_SOM_MONITOR + 2\15, 0, 0, 0 ; wrong direction (none) for SOM monitor on line 0

; wrong line types for SOM punch reader

tpu0s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_SOM_PUNCHRD + 2\15, 0, 0, 0 ; wrong line type for SOM punch reader
tpu1s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_SOM_PUNCHRD + 2\15, 0, 0, 0 ; wrong line type for SOM punch reader
tpu2s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_HD + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_SOM_PUNCHRD + 2\15, 0, 0, 0 ; wrong line type for SOM punch reader

; wrong line types for SOM puncher

tpr0s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_PUNCHER + 2\15, 0, 0, 0 ; wrong line type for SOM puncher
tpr1s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_PUNCHER + 2\15, 0, 0, 0 ; wrong line type for SOM puncher
tpr2s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_HD + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_SOM_PUNCHER + 2\15, 0, 0, 0 ; wrong line type for SOM puncher

; wrong line types for SOM terminal

tm0s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_SOM_MONITOR + 2\15, 0, 0, 0 ; wrong line type for SOM monitor
tm1s:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_HD + MX_LINE_USED + MX_LTYPE_USARTS + 3
	.word	MX_LPROTO_SOM_MONITOR + 2\15, 0, 0, 0 ; wrong line type for SOM monitor

; ---------------------------------------------------------------
; correct configuration
; ---------------------------------------------------------------

cfgok:	.word	13\7 + 22\15, 0
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTA + 0		; 0
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_USARTA + 0	; 1
	.word	MX_LDIR_FD + MX_LINE_USED + MX_LTYPE_USARTA + 2		; 2, 3, 4
	.word	MX_LDIR_HD + MX_LINE_USED + MX_LTYPE_USARTA + 2		; 5, 6, 7
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_8255 + 1		; 8, 9
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_8255 + 1		; 10, 11
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_TAPE + 3		; 12, 13, 14, 15
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_FLOPPY + 3	; 16, 17, 18, 19
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_WINCH + 3	; 20, 21, 22, 23
	.word	MX_LDIR_IN + MX_LINE_USED + MX_LTYPE_USARTA + 0		; 24
	.word	MX_LDIR_OUT + MX_LINE_USED + MX_LTYPE_USARTA + 0	; 25
	.word	MX_LDIR_FD + MX_LINE_USED + MX_LTYPE_USARTA + 2		; 26, 27, 28
	.word	MX_LDIR_HD + MX_LINE_USED + MX_LTYPE_USARTA + 2		; 29, 30, 31

	.word	MX_LPROTO_PUNCHRD + 0, 0, 0, 0
	.word	MX_LPROTO_PUNCHER + 1, 0, 0, 0
	.word	MX_LPROTO_PUNCHRD + 2, 0, 0, 0
	.word	MX_LPROTO_PUNCHER + 3, 0, 0, 0
	.word	MX_LPROTO_MONITOR + 4, 0, 0, 0
	.word	MX_LPROTO_PUNCHRD + 5, 0, 0, 0
	.word	MX_LPROTO_PUNCHER + 6, 0, 0, 0
	.word	MX_LPROTO_MONITOR + 7, 0, 0, 0
	.word	MX_LPROTO_PUNCHRD + 8, 0, 0, 0
;	.word	MX_LPROTO_SOM_PUNCHRD + 9, 0, 0, 0
	.word	MX_LPROTO_PUNCHER + 10, 0, 0, 0
;	.word	MX_LPROTO_SOM_PUNCHER + 11, 0, 0, 0
	.word	MX_LPROTO_TAPE + 0\8 + 12, 0, 0, 0
	.word	MX_LPROTO_TAPE + 1\8 + 13, 0, 0, 0
	.word	MX_LPROTO_TAPE + 0\8 + 14, 0, 0, 0
	.word	MX_LPROTO_TAPE + 1\8 + 15, 0, 0, 0
	.word	MX_LPROTO_FLOPPY + 16, MX_FLOPPY_SD, 0, 0
	.word	MX_LPROTO_FLOPPY + 17, MX_FLOPPY_DD, 0, 0
	.word	MX_LPROTO_FLOPPY + 18, MX_FLOPPY_HD, 0, 0
	.word	MX_LPROTO_FLOPPY + 19, MX_FLOPPY_HD + MX_FORMAT_PROTECT, 0, 0
	.word	MX_LPROTO_WINCH + 20, 0\7 + MX_FORMAT_PROTECT, 0, 0
	.word	MX_LPROTO_WINCH + 21, 1\7 + MX_LONG_DISK_ADDR, 0, 0
	.word	MX_LPROTO_WINCH + 22, 3\7 + MX_FORMAT_PROTECT + MX_LONG_DISK_ADDR, 0, 0
	.word	MX_LPROTO_WINCH + 23, 7\7 + MX_FORMAT_PROTECT, 0, 0	; any winchester configuration is OK for MULTIX
;	.word	MX_LPROTO_SOM_PUNCHRD + 24, 0, 0, 0
;	.word	MX_LPROTO_SOM_PUNCHER + 25, 0, 0, 0
;	.word	MX_LPROTO_SOM_PUNCHRD + 26, 0, 0, 0
;	.word	MX_LPROTO_SOM_PUNCHER + 27, 0, 0, 0
;	.word	MX_LPROTO_SOM_MONITOR + 28, 0, 0, 0
;	.word	MX_LPROTO_SOM_PUNCHRD + 29, 0, 0, 0
;	.word	MX_LPROTO_SOM_PUNCHER + 30, 0, 0, 0
;	.word	MX_LPROTO_SOM_MONITOR + 31, 0, 0, 0

; ---------------------------------------------------------------
; reconfiguration of properly configured MULTIX
; ---------------------------------------------------------------

recfg:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_USED + MX_LTYPE_WINCH + 3
	.word	MX_LPROTO_WINCH + 0\15, 0, 0, 0

; ------------------------------------------------------------------------

; test blocks
; NOTE: all SOM protocols are disabled (MULTIX emulation don't support them atm)

	.const	test_size 3

seq:
	.word	dummy,	MX_IWYZE,	0
	.word	0x6666,	MX_INKOT,	0
;	.word	dpr0s,	MX_INKON,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpr1s,	MX_INKON,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpr2s,	MX_INKON,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpr3s,	MX_INKON,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpr4s,	MX_INKON,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpr5s,	MX_INKON,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpu0s,	MX_INKON,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpu1s,	MX_INKON,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpu2s,	MX_INKON,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpu3s,	MX_INKON,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpu4s,	MX_INKON,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dpu5s,	MX_INKON,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dm0s,	MX_INKON,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dm1s,	MX_INKON,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dm2s,	MX_INKON,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dm3s,	MX_INKON,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dm4s,	MX_INKON,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dm5s,	MX_INKON,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dm6s,	MX_INKON,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dm7s,	MX_INKON,	MX_SCERR_DIR_MISMATCH + 0
;	.word	dm8s,	MX_INKON,	MX_SCERR_DIR_MISMATCH + 0
;	.word	tpu0s,	MX_INKON,	MX_SCERR_TYPE_MISMATCH + 0
;	.word	tpu1s,	MX_INKON,	MX_SCERR_TYPE_MISMATCH + 0
;	.word	tpu2s,	MX_INKON,	MX_SCERR_TYPE_MISMATCH + 0
;	.word	tpr0s,	MX_INKON,	MX_SCERR_TYPE_MISMATCH + 0
;	.word	tpr1s,	MX_INKON,	MX_SCERR_TYPE_MISMATCH + 0
;	.word	tpr2s,	MX_INKON,	MX_SCERR_TYPE_MISMATCH + 0
;	.word	tm0s,	MX_INKON,	MX_SCERR_TYPE_MISMATCH + 0
;	.word	tm1s,	MX_INKON,	MX_SCERR_TYPE_MISMATCH + 0

	.word	cfgok,	MX_IUKON,	0
	.word	recfg,	MX_INKON,	MX_SCERR_CONFSET
seqe:

; ------------------------------------------------------------------------
; expects:
;  r1 - configuration field address
;  r4 - RJ return adress
setcfg:
	ou	r1, MX_CMD_SETCFG | 1\IO_CHAN
	.word	no, en, ok, pe
no:	hlt	041
ok:	uj	r4
en:	ujs	setcfg
pe:	hlt	042

; ------------------------------------------------------------------------
mx_proc:
	lw	r1, [STACKP]
	lw	r1, [r1-1]	; intspec from mx

	cw	r1, [r2+1]	; is intspec as expected?
	bb	r0, ?E
	hlt	040		; bad intspec

	cw	r1, MX_INKOT	; skip setconf result if INKOT
	jes	next

	lw	r1, [r2+2]	; load expected setconf return value
	lw	r3, [r2]
	lw	r5, [r3+1]
	cw	r1, r5		; load setconf return value
	bb	r0, ?E
	hlt	043		; bad setconf result

next:
	awt	r2, test_size	; next test
	cw	r2, seqe	; all tests finished?
	blc	?E
	hlt	077		; all tests OK

	lw	r1, [r2]
	rj	r4, setcfg

	lip

; ------------------------------------------------------------------------
start:
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, mx_proc
	rw	r1, INTV_CH1

	lw	r2, seq		; start of all tests
	im	msk_mx
loop:
	hlt
	ujs	loop

stack:

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT ir : 0xec3f
