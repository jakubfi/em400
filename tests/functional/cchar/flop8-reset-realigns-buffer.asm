; CONFIG configs/flop8_empty.ini

; check if a reset during block write resets the formatter buffer,
; so the next block write starts again at byte 0

	.cpu	mera400

	.include cpu.inc
	.include io.inc

	mcl
	uj	start

	.const	FLOP_CHAN 7
	.const	FLOP_DEV 2
	.const	FLOP FLOP_CHAN\IO_CHAN | FLOP_DEV\IO_DEV
	.const	MAGIC1 0x7c
	.const	MAGIC2 0xc5
	.const	PARTIAL_CNT 7
	.const	PARTIAL_CNT2 4

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
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, int_flop
	rw	r1, INTV_CH0 + FLOP_CHAN

	; first write is answered with EN

	ou	r1, FLOP | KZ_CMD_DEV_WRITE
	.word	.no, .en, .ok, .pe
.no:	hlt	040
.pe:	hlt	041
.ok:	hlt	042
.en:
	lw	r1, FLOP_DEV\KZ_INT_DEV + KZ_INT_DEVICE_READY\KZ_INT_NUM
	rw	r1, expected_int
	lj	waitint

	; write PARTIAL_CNT * MAGIC2 bytes to the buffer

partial:
	lw	r2, PARTIAL_CNT
.loop:
	lw	r1, MAGIC2
	ou	r1, FLOP | KZ_CMD_DEV_WRITE
	.word	.no, .en, .ok, .pe
.no:	hlt	050
.pe:	hlt	051
.en:	hlt	052
.ok:	drb	r2, .loop

	; reset - this should move the buffer pointer back to 0

reset:
	ou	r1, FLOP | KZ_CMD_DEV_RESET
	.word	.no, .en, .ok, .pe
.en:	hlt	053
.no:	hlt	054
.pe:	hlt	055
.ok:

	; overwrite the buffer (first write is answered with EN)

write:
	ou	r1, FLOP | KZ_CMD_DEV_WRITE
	.word	.no, .en, .ok, .pe
.ok:	hlt	056
.no:	hlt	057
.pe:	hlt	060
.en:
	lw	r1, FLOP_DEV\KZ_INT_DEV + KZ_INT_DEVICE_READY\KZ_INT_NUM
	rw	r1, expected_int
	lj	waitint

	; write PARTIAL_CNT2 * MAGIC1 bytes

write_block:
	lw	r2, PARTIAL_CNT2
.loop:
	lw	r1, MAGIC1
	ou	r1, FLOP | KZ_CMD_DEV_WRITE
	.word	.no, .en, .ok, .pe
.en:	hlt	061
.no:	hlt	062
.pe:	hlt	063
.ok:	drb	r2, .loop

	; detach should fill up the buffer with 0s and write the sector

detach:
	ou	r1, FLOP | KZ_CMD_DEV_DETACH
	.word	.no, .en, reposition, .pe
.no:	hlt	074
.pe:	hlt	075
.en:
	lw	r1, FLOP_DEV\KZ_INT_DEV + KZ_INT_DEVICE_READY\KZ_INT_NUM
	rw	r1, expected_int
	lj	waitint
	; DETACH has to finish with OK to continue with CTL commands
	ujs	detach

	; seek to track 1, sector 1

reposition:
	lw	r1, KZ_FLOPPY_DRIVE_0 | KZ_FLOPPY_SIDE_A | 1\KZ_FLOPPY_TRACK | 1\KZ_FLOPPY_SECTOR
	ou	r1, FLOP | KZ_CMD_CTL4
	.word	.no, .en, .ok, .pe
.en:	hlt	064
.no:	hlt	065
.pe:	hlt	066
.ok:

	; read the sector - first read answered EN

read:
	in	r1, FLOP | KZ_CMD_DEV_READ
	.word	.no, .en, .ok, .pe
.no:	hlt	070
.pe:	hlt	071
.ok:	hlt	067
.en:
	lw	r1, FLOP_DEV\KZ_INT_DEV + KZ_INT_DEVICE_READY\KZ_INT_NUM
	rw	r1, expected_int
	lj	waitint

	; read 128 bytes of the sector

read_block:
	lw	r2, 128
	lw	r3, buf<<1
.loop:
	in	r1, FLOP | KZ_CMD_DEV_READ
	.word	.no, .en, .ok, .pe
.no:	hlt	070
.pe:	hlt	071
.en:	hlt	072
.ok:	rb	r1, r3
	awt	r3, 1
	drb	r2, .loop

	; check sector contents: there should be no MAGIC2

check:
	lw	r1, 128
	lw	r2, buf<<1
	lwt	r3, 0
.loop:
	lb	r3, r2
	cw	r3, MAGIC2
	jes	fail
	awt	r2, 1
	drb	r1, .loop
	hlt	077
fail:
	hlt	073
	ujs	fail

buf:	.res	128
stack:

; XPCT ir : 0xec3f
; XPCT alarm : 0
