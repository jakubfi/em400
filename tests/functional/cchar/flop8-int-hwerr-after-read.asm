; OPTS -c configs/flop8-no-floppy.ini

; trying to read a byte when no floppy is inserter should return HW error

	.cpu	mera400

	.include cpu.inc
	.include io.inc

	mcl
	uj	start

	.const	FLOP_CHAN 7
	.const	FLOP_DEV 2
	.const	FLOP FLOP_CHAN\IO_CHAN | FLOP_DEV\IO_DEV
	.const	INT_READY 1
	.org	OS_START

; ------------------------------------------------------------------------
mask:	.word	IMASK_CH4_9

int_flop:
	md	[STACKP]
	lw	r1, [-SP_SPEC]
	cw	r1, FLOP_DEV\KZ_INT_DEV + KZ_INT_FAILURE\KZ_INT_NUM
	jes	.ok
	hlt	076
.ok:
	hlt	077

; ------------------------------------------------------------------------
; ------------------------------------------------------------------------
; ------------------------------------------------------------------------
start:
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, int_flop
	rw	r1, INTV_CH0 + FLOP_CHAN

	im	mask

	in	r7, FLOP | KZ_CMD_DEV_READ
	.word	.no1, .en1, .ok1, .pe1
.no1:	hlt	040
.en1:	ujs	.continue
.pe1:	hlt	041
.ok1:	hlt	042

.continue:
	hlt	0
	hlt	043

stack:

; XPCT ir : 0xec3f
; XPCT alarm : 0
