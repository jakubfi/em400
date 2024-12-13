; OPTS -c configs/flop8_empty.ini

; check how DETACH ends after a read

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
	lw	r7, [mask]
	im	mask
.hlt:	hlt	0
	ujs	.hlt

; ------------------------------------------------------------------------
; ------------------------------------------------------------------------
; ------------------------------------------------------------------------
start:
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, int_flop
	rw	r1, INTV_CH0 + FLOP_CHAN

	; first byte of the sector ends with EN

	in	r1, FLOP | KZ_CMD_DEV_READ
	.word	.no, .en, .ok, .pe
.no:	hlt	040
.pe:	hlt	041
.ok:	hlt	042
.en:
	lw	r1, FLOP_DEV\KZ_INT_DEV + INT_READY\KZ_INT_NUM
	rw	r1, expected_int
	lj	waitint

	; read byte with OK
write:
	in	r1, FLOP | KZ_CMD_DEV_READ
	.word	.no, .en, .ok, .pe
.no:	hlt	050
.pe:	hlt	051
.en:	hlt	052
.ok:

	; detach ends with EN

detach:
	ou	r1, FLOP | KZ_CMD_DEV_DETACH
	.word	.no, .en, .ok, .pe
.no:	hlt	060
.ok:	hlt	061
.pe:	hlt	062
.en:

	; delay next detach

	lw	r2, 10
	lw	r1, 0
.loop1:	drb	r1, .loop1
	drb	r2, .loop1

	; next detach should be in IDLE state and end with OK

detach2:
	ou	r1, FLOP | KZ_CMD_DEV_DETACH
	.word	.no, .en, .ok, .pe
.no:	hlt	070
.pe:	hlt	071
.en:	hlt	072
.ok:

	hlt	077

stack:

; XPCT ir : 0xec3f
; XPCT alarm : 0
