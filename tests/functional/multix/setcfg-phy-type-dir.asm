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
; physical lines - wrong line type vs. direction combinations
; ---------------------------------------------------------------

dummy:	.word	0, 0 ; dummy to acknowledge IWYZE
pdir0:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_NONE + MX_LINE_UNUSED + MX_LTYPE_8255 + 0 ; wrong 8255 line 0 direction (none)
pdir1:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_8255 + 0 ; wrong 8255 line 0 direction (full-duplex)
pdir2:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_HD + MX_LINE_UNUSED + MX_LTYPE_8255 + 0 ; wrong 8255 line 0 direction (half-duplex)
wdir0:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_UNUSED + MX_LTYPE_WINCH + 0 ; wrong winchester line 0 direction (input)
wdir1:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_UNUSED + MX_LTYPE_WINCH + 0 ; wrong winchester line 0 direction (output)
wdir2:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_HD + MX_LINE_UNUSED + MX_LTYPE_WINCH + 0 ; wrong winchester line 0 direction (half-duplex)
wdir3:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_WINCH + 0 ; wrong winchester line 0 direction (full-duplex)
fdir0:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_UNUSED + MX_LTYPE_FLOPPY + 0 ; wrong floppy line 0 direction (input)
fdir1:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_UNUSED + MX_LTYPE_FLOPPY + 0 ; wrong floppy line 0 direction (output)
fdir2:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_HD + MX_LINE_UNUSED + MX_LTYPE_FLOPPY + 0 ; wrong floppy line 0 direction (half-duplex)
fdir3:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_FLOPPY + 0 ; wrong floppy line 0 direction (full-duplex)
tdir0:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_IN + MX_LINE_UNUSED + MX_LTYPE_TAPE + 0 ; wrong tape line 0 direction (input)
tdir1:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_OUT + MX_LINE_UNUSED + MX_LTYPE_TAPE + 0 ; wrong tape line 0 direction (output)
tdir2:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_HD + MX_LINE_UNUSED + MX_LTYPE_TAPE + 0 ; wrong tape line 0 direction (half-duplex)
tdir3:	.word	1\7 + 1\15, 0
	.word	MX_LDIR_FD + MX_LINE_UNUSED + MX_LTYPE_TAPE + 0 ; wrong tape line 0 direction (full-duplex)

	.const	test_size 3

seq:
	.word	dummy,	MX_IWYZE,	0
	.word	pdir0,	MX_INKON,	MX_SCERR_DIR + 0
	.word	pdir1,	MX_INKON,	MX_SCERR_DIR + 0
	.word	pdir2,	MX_INKON,	MX_SCERR_DIR + 0
	.word	wdir0,	MX_INKON,	MX_SCERR_DIR + 0
	.word	wdir1,	MX_INKON,	MX_SCERR_DIR + 0
	.word	wdir2,	MX_INKON,	MX_SCERR_DIR + 0
	.word	wdir3,	MX_INKON,	MX_SCERR_DIR + 0
	.word	fdir0,	MX_INKON,	MX_SCERR_DIR + 0
	.word	fdir1,	MX_INKON,	MX_SCERR_DIR + 0
	.word	fdir2,	MX_INKON,	MX_SCERR_DIR + 0
	.word	fdir3,	MX_INKON,	MX_SCERR_DIR + 0
	.word	tdir0,	MX_INKON,	MX_SCERR_DIR + 0
	.word	tdir1,	MX_INKON,	MX_SCERR_DIR + 0
	.word	tdir2,	MX_INKON,	MX_SCERR_DIR + 0
	.word	tdir3,	MX_INKON,	MX_SCERR_DIR + 0
seqe:

; ------------------------------------------------------------------------
; expects:
;  r1 - configuration field address
;  r4 - RJ return adress
setcfg:
	ou	r1, MX_IO_SETCFG | 1\IO_CHAN
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
