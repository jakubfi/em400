; OPTS -c configs/flop8_empty.ini

; validate operation sequence when reading sectors with no address set:
; read - EN - interrupt - read - ok - read - ok (x128) - read - EN - read - OK - ...
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
expected_int:
	.res	1

int_flop:
	md	[STACKP]
	lw	r7, [-SP_SPEC]
	cw	r7, [expected_int]
	jes	.ok
	hlt	076
.ok:
	lw	r7, [waitint]
	md	[STACKP]
	rw	r7, -SP_IC
	lwt	r7, 0
	md	[STACKP]
	rw	r7, -SP_SR
	lip

; ------------------------------------------------------------------------
waitint:
	.res	1
	im	mask
.hlt:	hlt	0
	ujs	.hlt

; ------------------------------------------------------------------------
; ------------------------------------------------------------------------
; ------------------------------------------------------------------------
start:
	mcl
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, int_flop
	rw	r1, INTV_CH0 + FLOP_CHAN

	lw	r3, 2 + 1	; read two sectors
next_sector:
	awt	r3, -1
	cwt	r3, 0
	jes	test_done
	; first byte of the sector ends with EN
	in	r1, FLOP | KZ_CMD_DEV_READ
	.word	.no1, .en1, .ok1, .pe1
.no1:	hlt	040
.pe1:	hlt	041
.ok1:	hlt	042
.en1:
	lw	r1, FLOP_DEV\KZ_INT_DEV + INT_READY\KZ_INT_NUM
	rw	r1, expected_int
	lj	waitint

	lw	r2, 128 + 1
next_byte:
	awt	r2, -1
	cwt	r2, 0
	jes	sector_done
	; 128 consecutive reads end with OK
	in	r1, FLOP | KZ_CMD_DEV_READ
	.word	.no2, .en2, .ok2, .pe2
.no2:	hlt	050
.pe2:	hlt	051
.ok2:	ujs	next_byte
.en2:	hlt	052

sector_done:
	ujs	next_sector

test_done:
	hlt	077
stack:

; XPCT ir : 0xec3f
; XPCT alarm : 0
