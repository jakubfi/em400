; OPTS -c configs/flop8_empty.ini

; check if DETACH command fills sector being written with 0's

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

pe:	hlt	075
no:	hlt	076

	.const	MAGIC1 0xa5
	.const	MAGIC2 0x7c
	.const	MAGIC2_CNT 7

; ------------------------------------------------------------------------
; ------------------------------------------------------------------------
; ------------------------------------------------------------------------
start:
	mcl

	lw	r1, stack
	rw	r1, STACKP
	lw	r1, int_flop
	rw	r1, INTV_CH0 + FLOP_CHAN

	; writing first byte of the sector ends with EN

	ou	r1, FLOP | KZ_CMD_DEV_WRITE
	.word	no, .en, .ok, pe
.ok:	hlt	042
.en:
	lw	r1, FLOP_DEV\KZ_INT_DEV + INT_READY\KZ_INT_NUM
	rw	r1, expected_int
	lj	waitint

	; write 128 bytes MAGIC1

	lw	r2, 128 + 1
next_byte:
	awt	r2, -1
	cwt	r2, 0
	jes	detach
	lw	r1, MAGIC1
	ou	r1, FLOP | KZ_CMD_DEV_WRITE
	.word	no, .en, .ok, pe
.ok:	ujs	next_byte
.en:	hlt	052

	; reposition again on track 1, sector 1

detach:
	ou	r1, FLOP | KZ_CMD_DEV_DETACH
	.word	no, .en, reposition, pe
.en:
	lw	r1, FLOP_DEV\KZ_INT_DEV + INT_READY\KZ_INT_NUM
	rw	r1, expected_int
	lj	waitint

reposition:
	lw	r1, KZ_FLOPPY_DRIVE_0 | KZ_FLOPPY_SIDE_A | 1\KZ_FLOPPY_TRACK | 1\KZ_FLOPPY_SECTOR
	ou	r1, FLOP | KZ_CMD_CTL1
	.word	no, .en, .ok, pe
.en:	hlt	071
.ok:

	; writing first byte of the sector ends with EN

write:
	lw	r1, MAGIC2
	ou	r1, FLOP | KZ_CMD_DEV_WRITE
	.word	no, .en, .ok, pe
.ok:	hlt	042
.en:
	lw	r1, FLOP_DEV\KZ_INT_DEV + INT_READY\KZ_INT_NUM
	rw	r1, expected_int
	lj	waitint

	; write 7 bytes 0x3a

	lw	r2, MAGIC2_CNT + 1
next_byte_0x3a:
	awt	r2, -1
	cwt	r2, 0
	jes	check
	lw	r1, MAGIC2
	ou	r1, FLOP | KZ_CMD_DEV_WRITE
	.word	no, .en, .ok, pe
.ok:	ujs	next_byte_0x3a
.en:	hlt	052

	; detach

check:
detach2:
	ou	r1, FLOP | KZ_CMD_DEV_DETACH
	.word	no, .en, reposition2, pe
.en:
	lw	r1, FLOP_DEV\KZ_INT_DEV + INT_READY\KZ_INT_NUM
	rw	r1, expected_int
	lj	waitint

	; reposition

reposition2:
	lw	r1, KZ_FLOPPY_DRIVE_0 | KZ_FLOPPY_SIDE_A | 1\KZ_FLOPPY_TRACK | 1\KZ_FLOPPY_SECTOR
	ou	r1, FLOP | KZ_CMD_CTL1
	.word	no, .en, .ok, pe
.en:	hlt	071
.ok:

	; read first byte (EN)

read:
	in	r1, FLOP | KZ_CMD_DEV_READ
	.word	no, .en, .ok, pe
.ok:	hlt	072
.en:
	lw	r1, FLOP_DEV\KZ_INT_DEV + INT_READY\KZ_INT_NUM
	rw	r1, expected_int
	lj	waitint

	; read 128 bytes (OK)

read128:

	lw	r2, 128 + 1
	lw	r3, buf<<1
.next_byte:
	awt	r2, -1
	cwt	r2, 0
	jes	check_magic2
	in	r1, FLOP | KZ_CMD_DEV_READ
	.word	no, .en, .ok, pe
.ok:	rb	r1, r3
	awt	r3, 1
	ujs	.next_byte
.en:	hlt	073

	; first MAGIC2_CNT bytes should be MAGIC2

check_magic2:
	lw	r1, MAGIC2_CNT
	lw	r2, buf<<1
	lwt	r3, 0
.loop:
	lb	r3, r2
	lw	r3, r3
	cw	r3, MAGIC2
	jes	.cont
	hlt	076
.cont:
	awt	r2, 1
	awt	r1, -1
	cwt	r1, 0
	jes	check_magic1
	ujs	.loop

	; rest of the sector should contain 0's

check_magic1:
	lw	r1, 128 - MAGIC2_CNT
.loop:
	lb	r3, r2
	lw	r3, r3
	cw	r3, 0
	jes	.cont
	hlt	076
.cont:
	awt	r2, 1
	awt	r1, -1
	cwt	r1, 0
	jes	done
	ujs	.loop

done:
	hlt	077

buf:	.res	128
stack:

; XPCT ir : 0xec3f
; XPCT alarm : 0
