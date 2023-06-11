; OPTS -c configs/flop8_prewrite.ini

; read floppy contents with positioning (not using interruts)

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
; r5 - expected logical address
; each sector in the test image contains:
;  - on first two bytes: word with logical sector addres (starting from 0)
;  - subsequent bytes: byte number (starting from 2)
check_sector:
	.res	1

	lwt	r0, 0	; flag to indicate which byte is processed
	lwt	r1, 0
.addr_loop:
	in	r7, FLOP | KZ_CMD_DEV_READ
	.word	.no1, .en1, .ok1, .pe1
.no1:	hlt	040
.en1:	ujs	.addr_loop
.pe1:	hlt	041
.ok1:	zlb	r7
	or	r1, r7
	jxs	.check_addr
	xr	r0, ?X
	shc	r1, 8
	ujs	.addr_loop

.check_addr:
	cw	r1, r5
	jes	.check_sector_bytes
	hlt	050

.check_sector_bytes:
	lwt	r3, 2	; byte numbers start from 2

.data_loop:
	in	r2, FLOP | KZ_CMD_DEV_READ
	.word	.no2, .en2, .ok2, .pe2
.no2:	hlt	042
.en2:	ujs	.data_loop
.pe2:	hlt	043
.ok2:	cw	r2, r3
	jes	.next_byte
	hlt	51

.next_byte:
	awt	r3, 1
	cw	r3, 128
	jes	.end
	ujs	.data_loop

.end:
	uj	[check_sector]

; ------------------------------------------------------------------------
; check if positioning for read works
positioned_read:
	.res	1

	lw	r6, .positions

.loop:
	; detach drive
	ou	r2, FLOP | KZ_CMD_DEV_DETACH
	.word	.no1, .en1, .ok1, .pe1
.no1:	hlt	45
.en1:	hlt	46
.pe1:	hlt	47
.ok1:
.position:
	; position head
	lw	r2, [r6]
	ou	r2, FLOP | KZ_CMD_CTL4
	.word	.no2, .en2, .ok2, .pe2
.no2:	hlt	062
.en2:	ujs	.position
.pe2:	hlt	063
.ok2:
	; check sector
	lw	r5, [r6+1]
	lj	check_sector
	awt	r6, 2
	lwt	r5, 0
	cw	r5, [r6]
	jes	.end
	ujs	.loop

.end:
	uj	[positioned_read]
.positions:
	;	physical sector address		logical sector number
	.word	0\1 | 0\2 | 0\3 |  0\10 | 1,	0
	.word	0\1 | 0\2 | 0\3 |  0\10 | 2,	1
	.word	0\1 | 0\2 | 0\3 |  1\10 | 1,	26
	.word	0\1 | 0\2 | 0\3 | 76\10 | 1,	76*26
	.word	0\1 | 0\2 | 0\3 | 76\10 | 26,	77*26 - 1

	.word	0\1 | 1\2 | 0\3 |  0\10 | 1,	77*26
	.word	0\1 | 1\2 | 0\3 |  0\10 | 2,	77*26 + 1
	.word	0\1 | 1\2 | 0\3 |  1\10 | 1,	77*26 + 26
	.word	0\1 | 1\2 | 0\3 | 76\10 | 1,	77*26 + 76*26
	.word	0\1 | 1\2 | 0\3 | 76\10 | 26,	77*26 + 77*26 - 1

	.word	2\1 | 0\2 | 0\3 |  0\10 | 1,	2*77*26
	.word	2\1 | 0\2 | 0\3 |  0\10 | 2,	2*77*26 + 1
	.word	2\1 | 0\2 | 0\3 |  1\10 | 1,	2*77*26 + 26
	.word	2\1 | 0\2 | 0\3 | 76\10 | 1,	2*77*26 + 76*26
	.word	2\1 | 0\2 | 0\3 | 76\10 | 26,	2*77*26 + 77*26 - 1

	.word	2\1 | 1\2 | 0\3 |  0\10 | 1,	3*77*26
	.word	2\1 | 1\2 | 0\3 |  0\10 | 2,	3*77*26 + 1
	.word	2\1 | 1\2 | 0\3 |  1\10 | 1,	3*77*26 + 26
	.word	2\1 | 1\2 | 0\3 | 76\10 | 1,	3*77*26 + 76*26
	.word	2\1 | 1\2 | 0\3 | 76\10 | 26,	3*77*26 + 77*26 - 1
	.word	0

; ------------------------------------------------------------------------
start:
	lj	positioned_read

	hlt	077

; XPCT ir : 0xec3f
; XPCT alarm : 0
