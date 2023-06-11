; OPTS -c configs/flop8_empty.ini

; control after read results in EN

	.cpu	mera400

	.include cpu.inc
	.include io.inc

	mcl
	uj	start

	.const	FLOP_CHAN 7
	.const	FLOP_DEV 2
	.const	FLOP FLOP_CHAN\IO_CHAN | FLOP_DEV\IO_DEV
	.org	OS_START

commands:
	.word	FLOP | KZ_CMD_CTL1
	.word	FLOP | KZ_CMD_CTL2
	.word	FLOP | KZ_CMD_CTL3
	.word	FLOP | KZ_CMD_CTL4
	.word	0

; ------------------------------------------------------------------------
; ------------------------------------------------------------------------
; ------------------------------------------------------------------------

start:

read:
	; reading first byte of the sector results in EN

	in	r1, FLOP | KZ_CMD_DEV_READ
	.word	.no, .en, .ok, .pe
.no:	hlt	040
.pe:	hlt	041
.ok:	hlt	042
.en:

	lw	r7, KZ_FLOPPY_DRIVE_0 | KZ_FLOPPY_SIDE_A | 1\KZ_FLOPPY_TRACK | 1\KZ_FLOPPY_SECTOR
	lw	r2, commands

control:

	; every control command now fails with EN

	lw	r3, [r2]
	cwt	r3, 0
	jes	done

	ou	r7, r3
	.word	.no, .en, .ok, .pe
.no:	hlt	050
.pe:	hlt	051
.ok:	ujs	052
.en:	
	awt	r2, 1
	ujs	control
done:
	hlt	077

; XPCT ir : 0xec3f
; XPCT alarm : 0
