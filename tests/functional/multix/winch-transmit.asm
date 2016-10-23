; OPTS -c configs/winchester.cfg

	.cpu	mx16

	.include hw.inc
	.include io.inc
	.include mx.inc

	; use bit 15 of the register I/O argument for IN/OU selection
	.const IO_IN 1
	.const IO_OU 0

	; winchester device address
	.const	WINCH_LINE 2
	.const	WINCH_ADDR MX_CHAN_DEFAULT | WINCH_LINE\10

	; MULTIX command shortcuts
	.const	MXCMD_SETCFG	MX_IO_SETCFG | MX_CHAN_DEFAULT | IO_OU
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

	UJ	start

msk_0:	.word	IMASK_NONE
msk_tm:	.word	IMASK_CPU
msk_mx:	.word	IMASK_CH0_1 | IMASK_CPU
xlip:	LIP

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
	.word 0
tmp_r7:	.res 1
mx_proc:
	RW	r7, tmp_r7
	MD	[STACKP]
	LW	r7, [-1]
	RW	r7, mx_last_int
	LW	r7, [tmp_r7]
	LIP

; ------------------------------------------------------------------------
; I/O handler
; expects:
;  r1 - I/O command + IN/OU information on bit 15
;  r2 - configuration field address
;  r3 - expected interrupt specification
;  r4 - RJ return adress
io_cmd:
	SXL	r1
	ER	r1, 1
	RZ	mx_last_int
repeat:	JXS	c_in
c_ou:	OU	r2, r1
	.word	c_no, c_en, c_ok, c_pe
c_in:	IN	r2, r1
	.word	c_no, c_en, c_ok, c_pe
c_no:	HLT	041	; error
c_en:	UJS	repeat	; repeat if engaged
c_pe:	HLT	042	; error
c_ok:	LW	r1, [mx_last_int]
	NR	r1, r1
	BB	r0, ?Z	; multix interrupt ready?
	UJS	c_ret	; yes
	HLT		; no -> wait
	UJS	c_ok
c_ret:	CW	r3, r1
	BB	r0, ?E	; intspec as expected?
	HLT	043
	UJ	r4

; ------------------------------------------------------------------------
; sector addressing test
; read the whole disk (sectors 16..USER_SECTORS to be precise)
; and check if every word of each sector contains the sector number
reada:	.word	2\7 + 0\15, rdbuf, 255, 0
readas:	.word	0, -1, -1
readat:	.word	MXCMD_TRANSMIT, reada, MX_IETRA\7 + WINCH_LINE
rdsect:
	.res	1
	.const	START_SECT 16
	LW	r6, START_SECT
rdloop:
	RW	r6, readas			; update sector number in control field
	LF	readat
	RJ	r4, io_cmd			; read sector

readok:	; check sector contents
	LW	r1, rdbuf-1
	LW	r3, SSIZE
rdc_loop:
        LW      r4, [r1+r3]		; load word
        CW      r4, r6			; compare to sector number
        BB      r0, ?E
        HLT     050
        DRB     r3, rdc_loop		; next word
	; contents OK
	AWT	r6, 1			; next sector
	CW	r6, USER_SECTORS	; last sector?
	JN	rdloop			; no -> loop over
	UJ	[rdsect]

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
	.word	MXCMD_SETCFG, conf, MX_IUKON\7, 0
	.word	MXCMD_ATTACH, -1, MX_IDOLI\7 + WINCH_LINE, 0
	.word	MXCMD_TRANSMIT, park, MX_IETRA\7 + WINCH_LINE, 0
	.word	MXCMD_TRANSMIT, spare, MX_IETRA\7 + WINCH_LINE, 0
	.word	MXCMD_TRANSMIT, rdfail, MX_INTRA\7 + WINCH_LINE, 0
	.word	MXCMD_TRANSMIT, rdfail2, MX_INPAO\7 + WINCH_LINE, 0
	.word	MXCMD_TRANSMIT, -1, MX_INPAO\7 + WINCH_LINE, 0
	.word	MXCMD_TRANSMIT, write, MX_IETRA\7 + WINCH_LINE, 0
	.word	MXCMD_TRANSMIT, format, MX_IETRA\7 + WINCH_LINE, 0
	.word	MXCMD_TRANSMIT, read, MX_IETRA\7 + WINCH_LINE, cmpz
	.word	MXCMD_TRANSMIT, write, MX_IETRA\7 + WINCH_LINE, 0
	.word	MXCMD_TRANSMIT, read, MX_IETRA\7 + WINCH_LINE, cmpbuf
	.word	MXCMD_ABORT, -1, MX_INABT\7 + WINCH_LINE, rdsect
	.word	MXCMD_DETACH, -1, MX_IODLI\7 + WINCH_LINE, 0
seqe:

; ------------------------------------------------------------------------
; check if rdbuf is zeroes only
cmpz:
	.res	1
	LW	r1, rdbuf-1
	LW	r3, SSIZE
cmpz_loop:
	LW	r4, [r1+r3]
	CW	r4, 0
	BB	r0, ?E
	HLT	051
	DRB	r3, cmpz_loop
	UJ	[cmpz]

; ------------------------------------------------------------------------
; compare contents of rdbuf with wrbuf
cmpbuf:
	.res	1
	LW	r1, wrbuf-1
	LW	r2, rdbuf-1
	LW	r3, SSIZE
cmp_loop:
	LW	r4, [r1+r3]
	CW	r4, [r2+r3]
	BB	r0, ?E
	HLT	052
	DRB	r3, cmp_loop
	UJ	[cmpbuf]

; ------------------------------------------------------------------------
; ---- MAIN --------------------------------------------------------------
; ------------------------------------------------------------------------
start:
	LW	r1, stack
	RW	r1, STACKP
	LW	r1, mx_proc
	RW	r1, MX_IV

	; seed prng
	IM	msk_tm
	LJ	prngseed

	IM	msk_mx

fill:	; fill write buffer with random data
	LW	r4, wrbuf
	LW	r3, SSIZE/2
fill_loop:
	LJ	rand
	RD	r4
	AWT	r4, 2
	DRB	r3, fill_loop

mxinit:	; wait for MX initialization to end
	LW	r1, [mx_last_int]
	CW	r1, MX_IWYZE\7
	JES	run_tests
	HLT
	UJS	mxinit

run_tests:
	; test loop
	LW	r7, seq
next_test:
	LF	r7
	RJ	r4, io_cmd
	LW	r1, [r7+3]
	CW	r1, 0
	JES	no_check_proc
	LJ	r1
no_check_proc:
	AWT	r7, 4
	CW	r7, seqe
	JN	next_test

	HLT	077

; ------------------------------------------------------------------------
; buffers
stack:	.res	16*4
rdbuf:	.res	SSIZE
wrbuf:	.res	SSIZE

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT ir&0x3f : 0o77
