; OPTS -c configs/flop8_prewrite.ini

; read flopy contents without setting initial address and verify results
; (without using interrupts)

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
start:
	lj	read_after_reset

	hlt	077

; XPCT ir : 0xec3f
; XPCT alarm : 0
