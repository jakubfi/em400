; OPTS -c configs/flop8_empty.ini

; several detach commands in idle state is OK

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
	hlt	076

commands:
	.word	KZ_CMD_DEV_DETACH, KZ_CMD_DEV_DETACH, KZ_CMD_DEV_DETACH
	.word	KZ_CMD_DEV_DETACH, KZ_CMD_DEV_DETACH, KZ_CMD_DEV_DETACH
	.word	0

; ------------------------------------------------------------------------
; ------------------------------------------------------------------------
; ------------------------------------------------------------------------
start:
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, int_flop
	rw	r1, INTV_CH0 + FLOP_CHAN

	im	mask

	lw	r2, commands
.loop:
	lw	r3, [r2]
	cwt	r3, 0
	jes	.fin

	md	r3
	ou	r7, FLOP
	.word	.no1, .en1, .ok1, .pe1
.no1:	hlt	040
.en1:	ujs	041
.pe1:	hlt	042
.ok1:	
	awt	r2, 1
	ujs	.loop

.fin:
	hlt	077

stack:

; XPCT ir : 0xec3f
; XPCT alarm : 0
