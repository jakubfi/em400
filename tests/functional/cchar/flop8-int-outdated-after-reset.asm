; CONFIG configs/flop8_empty.ini

; reset between an interrupt report and the specification fetch
; makes the unit answer with "outdated interrupt" (00000)

	.cpu	mera400

	.include cpu.inc
	.include io.inc

	mcl
	uj	start

	.const	FLOP_CHAN 7
	.const	FLOP_DEV 2
	.const	FLOP FLOP_CHAN\IO_CHAN | FLOP_DEV\IO_DEV
	.org	OS_START

; ------------------------------------------------------------------------
mask:	.word	IMASK_CH4_9

int_flop:
	md	[STACKP]
	lw	r1, [-SP_SPEC]
	cw	r1, FLOP_DEV\KZ_INT_DEV + KZ_INT_INVALID\KZ_INT_NUM
	jes	.ok
	hlt	060
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

	; interrupts stay masked for now, first write ends with EN

	ou	r7, FLOP | KZ_CMD_DEV_WRITE
	.word	.no, .en, .ok, .pe
.no:	hlt	040
.pe:	hlt	041
.ok:	hlt	042
.en:

	; give the interrupt some time

	lw	r2, 30000
.loop:
	drb	r2, .loop

	; send reset before fetching the interrupt

reset:
	ou	r7, FLOP | KZ_CMD_DEV_RESET
	.word	.no, .en, .ok, .pe
.no:	hlt	045
.pe:	hlt	046
.en:	hlt	047
.ok:

	; now let the interrupt come through
	im	mask

.hltloop:
	hlt	050
	ujs .hltloop

stack:

; XPCT ir : 0xec3f
; XPCT alarm : 0
