; OPTS -c configs/flop8_empty.ini
; PRECMD CLOCK ON

	.cpu	mera400

	.include cpu.inc
	.include io.inc

	mcl
	uj	start

	.const	FLOP_CHAN 7
	.const	FLOP_DEV 2
	.const	FLOP FLOP_CHAN\IO_CHAN | FLOP_DEV\IO_DEV

	.org	OS_START

	.include prng.inc

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
write_positions:
	;	physical sector address		generated random
	.word	0\1 | 0\2 | 0\3 |  0\10 | 1,	0
	.word	0\1 | 0\2 | 0\3 |  0\10 | 26,	0
	.word	0\1 | 0\2 | 0\3 | 76\10 | 1,	0
	.word	0\1 | 0\2 | 0\3 | 76\10 | 26,	0

	.word	0\1 | 1\2 | 0\3 |  0\10 | 1,	0
	.word	0\1 | 1\2 | 0\3 |  0\10 | 26,	0
	.word	0\1 | 1\2 | 0\3 | 76\10 | 1,	0
	.word	0\1 | 1\2 | 0\3 | 76\10 | 26,	0

	.word	2\1 | 0\2 | 0\3 |  0\10 | 1,	0
	.word	2\1 | 0\2 | 0\3 |  0\10 | 26,	0
	.word	2\1 | 0\2 | 0\3 | 76\10 | 1,	0
	.word	2\1 | 0\2 | 0\3 | 76\10 | 26,	0

	.word	2\1 | 1\2 | 0\3 |  0\10 | 1,	0
	.word	2\1 | 1\2 | 0\3 |  0\10 | 26,	0
	.word	2\1 | 1\2 | 0\3 | 76\10 | 1,	0
	.word	2\1 | 1\2 | 0\3 | 76\10 | 26,	0

	.word	0

; ------------------------------------------------------------------------
detach:
	.res	1
.retry:
	ou	r2, FLOP | KZ_CMD_DEV_DETACH
	.word	.no, .en, .ok, .pe
.no:	hlt	45
.en:	ujs	.retry
.pe:	hlt	47
.ok:
	uj	[detach]

; ------------------------------------------------------------------------
; check if writes work
positioned_write:
	.res	1

	lw	r6, write_positions

.loop:
	lj	detach
.write_pos:
	; position head
	lw	r2, [r6]
	ou	r2, FLOP | KZ_CMD_CTL1
	.word	.now1, .enw1, .okw1, .pew1
.now1:	hlt	044
.enw1:	ujs	.write_pos
.pew1:	hlt	045
.okw1:
	; write byte 1
	lw	r2, [r6+1]
	zlb	r2
	ou	r2, FLOP | KZ_CMD_DEV_WRITE
	.word	.now2, .enw2, .okw2, .pew2
.now2:	hlt	046
.enw2:	ujs	.okw1
.pew2:	hlt	047
.okw2:
	; write byte 2
	lw	r2, [r6+1]
	shc	r2, 8
	zlb	r2
	ou	r2, FLOP | KZ_CMD_DEV_WRITE
	.word	.now3, .enw3, .okw3, .pew3
.now3:	hlt	050
.enw3:	ujs	.okw2
.pew3:	hlt	051
.okw3:
	lj	detach
	; position head
	lw	r2, [r6]
	ou	r2, FLOP | KZ_CMD_CTL4
	.word	.nor1, .enr1, .okr1, .per1
.nor1:	hlt	052
.enr1:	ujs	.okw3
.per1:	hlt	053
.okr1:
	; read byte 1
	in	r2, FLOP | KZ_CMD_DEV_READ
	.word	.nor2, .enr2, .okr2, .per2
.nor2:	hlt	054
.enr2:	ujs	.okr1
.per2:	hlt	055
.okr2:
	lw	r3, [r6+1]
	zlb	r3
	cw	r3, r2
	jes	.read2
	hlt	056

.read2:
	; read byte 2
	in	r2, FLOP | KZ_CMD_DEV_READ
	.word	.nor3, .enr3, .okr3, .per3
.nor3:	hlt	057
.enr3:	ujs	.okr2
.per3:	hlt	060
.okr3:
	lw	r3, [r6+1]
	shc	r3, 8
	zlb	r3
	cw	r3, r2
	jes	.loop_over
	hlt	061

.loop_over:
	awt	r6, 2
	lwt	r7, 0
	cw	r7, [r6]
	jes	.end
	uj	.loop

.end:
	uj	[positioned_write]

; ------------------------------------------------------------------------
gen_randoms:
	.res	1

	lw	r6, write_positions
.loop:

	lj	rand
	rw	r1, r6+1

	awt	r6, 2
	lwt	r7, 0
	cw	r7, [r6]
	jes	.end
	ujs	.loop

.end:
	uj	[gen_randoms]

; ------------------------------------------------------------------------
start:
	lj	prngseed
	lj	gen_randoms
	lj	positioned_write

	hlt	077

; XPCT ir : 0xec3f
; XPCT alarm : 0
