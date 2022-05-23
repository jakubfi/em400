; OPTS -c configs/flop8_0.ini

	.cpu	mera400

	.include cpu.inc
	.include io.inc

	mcl
	uj	start

	.const	FLOP_CHAN 1
	.const	FLOP_DEV 0
	.const	FLOP FLOP_CHAN\IO_CHAN | FLOP_DEV\IO_DEV

	.org	OS_START

; ------------------------------------------------------------------------
; r5 - expected logical address
; each sector in the test image contains:
;  - on first two bytes: word with logical sector addres (starting from 0)
;  - subsequent bytes: byte number (starting from 2)
check_sector:
	.res	1

	lw	r0, ?X	; flag to indicate which byte is processed
	lwt	r1, 0
.addr_loop:
	xr	r0, ?X
	in	r7, FLOP | KZ_CMD_DEV_READ
	.word	.no1, .en1, .ok1, .pe1
.no1:	hlt	040
.en1:	ujs	.addr_loop
.pe1:	hlt	041
.ok1:	zlb	r7
	or	r1, r7
	jxs	.check_addr
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
; check if step-by-step sector addressing (starting from track 1 sector 1) works right after reset
read_after_reset:
	.res	1
	lw	r5, 26	; initially, head is positioned on track 1 sector 1 (logical sector 26)
.loop:
	lj	check_sector

.next_sector:
	awt	r5, 1
	cw	r5, 26*73	; sectors to check
	jes	.end
	ujs	.loop

.end:
	uj	[read_after_reset]

; ------------------------------------------------------------------------
; check if positioning for read works
positioned_read:
	.res	1

	lw	r6, .positions

.loop:
	; position head
	lw	r2, [r6]
	ou	r2, FLOP | KZ_CMD_CTL4
	.word	.no, .en, .ok, .pe
.no:	hlt	062
.en:	ujs	.loop
.pe:	hlt	063
.ok:
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
	lj	read_after_reset
	lj	positioned_read

	hlt	077

; XPCT ir : 0xec3f
; XPCT alarm : 0
