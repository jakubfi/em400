; OPTS -c configs/winchester.cfg
; PRECMD CLOCK ON

	.cpu	mx16

	.include hw.inc
	.include io.inc
	.include mx.inc

	; use bit 15 of the register I/O argument for IN/OU selection
	.const IO_IN 1
	.const IO_OU 0

	; winchester device address
	.const	WINCH_LINE 2
	.const	WINCH_ADDR MX_CHAN | WINCH_LINE\10

	; MULTIX command shortcuts
	.const	MXCMD_SETCFG	MX_IO_SETCFG | MX_CHAN | IO_OU
	.const	MXCMD_STATUS	MX_IO_STATUS | WINCH_ADDR | IO_OU
	.const	MXCMD_ATTACH	MX_IO_ATTACH | WINCH_ADDR | IO_OU
	.const	MXCMD_DETACH	MX_IO_DETACH | WINCH_ADDR | IO_IN
	.const	MXCMD_TRANSMIT	MX_IO_TRANSMIT | WINCH_ADDR | IO_OU
	.const	MXCMD_ABORT	MX_IO_ABORT | WINCH_ADDR | IO_IN

	; Winchester geometry
	.const	CYLINDERS 615
	.const	HEADS 4
	.const	SPT 16
	.const	SSIZE 256
	.const	TOTAL_SECTORS CYLINDERS*HEADS*SPT
	.const	USER_SECTORS TOTAL_SECTORS - 1*HEADS*SPT

	uj	start

msk_0:	.word	IMASK_NONE
msk_mx:	.word	IMASK_CH0_1 | IMASK_CPU
xlip:	lip

	.org	INTV
	.res	16, xlip	; dummy interrupt handlers
	.word	xlip		; dummy EXL handler

	.org	OS_MEM_BEG

	.include prng.inc

; ------------------------------------------------------------------------
; MULTIX interrupt handler
; updates:
;  mx_last_int
mx_last_int:
	.word	0
tmp_r7:	.res	1
mx_proc:
	rw	r7, tmp_r7
	md	[STACKP]
	lw	r7, [-1]
	rw	r7, mx_last_int
	lw	r7, [tmp_r7]
	lip

; ------------------------------------------------------------------------
; I/O handler
; expects:
;  r1 - I/O command + IN/OU information on bit 15
;  r2 - configuration field address
;  r3 - expected interrupt specification
;  r4 - RJ return adress
io_cmd:
	sxl	r1
	er	r1, 1
	rz	mx_last_int
repeat:	jxs	c_in
c_ou:	ou	r2, r1
	.word	c_no, c_en, c_ok, c_pe
c_in:	in	r2, r1
	.word	c_no, c_en, c_ok, c_pe
c_no:	hlt	041	; error
c_en:	ujs	repeat	; repeat if engaged
c_pe:	hlt	042	; error
c_ok:	lw	r1, [mx_last_int]
	nr	r1, r1
	bb	r0, ?Z	; multix interrupt ready?
	ujs	c_ret	; yes
	hlt		; no -> wait
	ujs	c_ok
c_ret:	cw	r3, r1
	bb	r0, ?E	; intspec as expected?
	ujs	c_fail
	uj	r4
c_fail:
	im	msk_0
	hlt	043

; ------------------------------------------------------------------------
; sector addressing test
; read the whole disk (sectors 16..USER_SECTORS to be precise)
; and check if every word of each sector contains the sector number
reada:	.word	2\7 + 0\15, rdbuf, 255, 0
readas:	.word	0, -1, -1
readat:	.word	MXCMD_TRANSMIT, reada, MX_IETRA + WINCH_LINE
rdsect:
	.res	1
	.const	START_SECT 16
	lw	r6, START_SECT
rdloop:
	rw	r6, readas			; update sector number in control field
	lf	readat
	rj	r4, io_cmd			; read sector

readok:	; check sector contents
	lw	r1, rdbuf-1
	lw	r3, SSIZE
rdc_loop:
	lw	r4, [r1+r3]		; load word
	cw	r4, r6			; compare to sector number
	bb	r0, ?E
	ujs	rdc_fail
	drb	r3, rdc_loop		; next word
	; contents OK
	awt	r6, 1			; next sector
	cw	r6, USER_SECTORS	; last sector?
	jn	rdloop			; no -> loop over
	uj	[rdsect]
rdc_fail:
	im	msk_0
	hlt	050

; ------------------------------------------------------------------------
; test data
conf:	.word	1\7 | 3\15, 0
	.word	MX_LDIR_NONE | MX_LINE_USED | MX_LTYPE_WINCH | 3
	.word	MX_LPROTO_WINCH | 2, 3\7 | MX_SHORT_DISK_ADDR | MX_NO_FORMAT_PROTECT, 0, 0
	.word	MX_LPROTO_WINCH | 1, 3\7 | MX_SHORT_DISK_ADDR | MX_NO_FORMAT_PROTECT, 0, 0
	.word	MX_LPROTO_WINCH | 0, 3\7 | MX_SHORT_DISK_ADDR | MX_NO_FORMAT_PROTECT, 0, 0
spare:	.word	0\7, 0, 0, 0, 0, -1, -1
park:	.word	5\7, 0, 0, 0, 644, -1, -1
format:	.word	1\7, 0, 0, 0, 0, -1, -1
write:	.word	3\7 + 0\15, wrbuf, 255, 0, 0, -1, -1
read:	.word	2\7 + 0\15, rdbuf, 255, 0, 0, -1, -1
rdfail:	.word	2\7 + 0\15, rdbuf, 255, 0, USER_SECTORS, -1, -1
rdfail2:.word	2\7 + 0\15, -1, 255, 0, 0, -1, -1

seq:	; [command, field_addr, exp_irq, check_proc]
	.word	MXCMD_SETCFG, conf, MX_IUKON, 0
	.word	MXCMD_ATTACH, -1, MX_IDOLI + WINCH_LINE, 0
	.word	MXCMD_TRANSMIT, park, MX_IETRA + WINCH_LINE, 0
	.word	MXCMD_TRANSMIT, spare, MX_IETRA + WINCH_LINE, 0
	.word	MXCMD_TRANSMIT, rdfail, MX_INTRA + WINCH_LINE, 0
	.word	MXCMD_TRANSMIT, rdfail2, MX_INPAO + WINCH_LINE, 0
	.word	MXCMD_TRANSMIT, -1, MX_INPAO + WINCH_LINE, 0
	.word	MXCMD_TRANSMIT, write, MX_IETRA + WINCH_LINE, 0
	.word	MXCMD_TRANSMIT, format, MX_IETRA + WINCH_LINE, 0
	.word	MXCMD_TRANSMIT, read, MX_IETRA + WINCH_LINE, cmpz
	.word	MXCMD_TRANSMIT, write, MX_IETRA + WINCH_LINE, 0
	.word	MXCMD_TRANSMIT, read, MX_IETRA + WINCH_LINE, cmpbuf
	.word	MXCMD_ABORT, -1, MX_INABT + WINCH_LINE, rdsect ; rdsect is here because it's convenient
	.word	MXCMD_DETACH, -1, MX_IODLI + WINCH_LINE, 0
seqe:

; ------------------------------------------------------------------------
; check if rdbuf is zeroes only
cmpz:
	.res	1
	lw	r1, rdbuf-1
	lw	r3, SSIZE
cmpz_loop:
	lw	r4, [r1+r3]
	cw	r4, 0
	bb	r0, ?E
	ujs	cmpz_fail
	drb	r3, cmpz_loop
	uj	[cmpz]
cmpz_fail:
	im	msk_0
	hlt	051

; ------------------------------------------------------------------------
; compare contents of rdbuf with wrbuf
cmpbuf:
	.res	1
	lw	r1, wrbuf-1
	lw	r2, rdbuf-1
	lw	r3, SSIZE
cmp_loop:
	lw	r4, [r1+r3]
	cw	r4, [r2+r3]
	bb	r0, ?E
	ujs	cmp_fail
	drb	r3, cmp_loop
	uj	[cmpbuf]
cmp_fail:
	im	msk_0
	hlt	052

; ------------------------------------------------------------------------
; ---- MAIN --------------------------------------------------------------
; ------------------------------------------------------------------------
start:
	lj	prngseed

	lw	r1, stack
	rw	r1, STACKP
	lw	r1, mx_proc
	rw	r1, MX_IV
	im	msk_mx

fill:	; fill write buffer with random data
	lw	r4, wrbuf
	lw	r3, SSIZE/2
fill_loop:
	lj	rand
	rd	r4
	awt	r4, 2
	drb	r3, fill_loop

mxinit:	; wait for MX initialization to end
	lw	r1, [mx_last_int]
	cw	r1, MX_IWYZE
	jes	run_tests
	hlt
	ujs	mxinit

run_tests:
	; test loop
	lw	r7, seq
next_test:
	lf	r7
	rj	r4, io_cmd
	lw	r1, [r7+3]
	cw	r1, 0
	jes	no_check_proc
	lj	r1
no_check_proc:
	awt	r7, 4
	cw	r7, seqe
	jn	next_test

	im	msk_0
	hlt	077

; ------------------------------------------------------------------------
; buffers
rdbuf:	.res	SSIZE
wrbuf:	.res	SSIZE
stack:

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT ir : 0xec3f
