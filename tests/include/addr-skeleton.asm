	.const	mega 1\15 + 1\6
	.const	dealloc 1\5
	.const	done 1\0
	.const	pshow 1\2
	.const	phide 1\1	

	uj	start

; ------------------------------------------------------------------------
mask:	.word	0b0100000000000000
stack:	.res	16

; ------------------------------------------------------------------------
; ignore interrupt
int_ignore:
	lip

; ------------------------------------------------------------------------
; stop test on interrupt
int_death:
	hlt	040

; ------------------------------------------------------------------------
	.res	0x40-.		; skip to interrupt vectors

	.word	int_ignore	; int: power failure
	.word	int_death	; int: memory parity
	.word	int_death	; int: no memory
	.res	29, int_ignore	; int: other interrupts
	.word	int_ignore	; exl
	.word	stack

; ------------------------------------------------------------------------
; configure memory
mem_cfg:
	.res	1
	lwt	r1, 2 ; AB, RAL
	lwt	r2, 0 ; NB, MP
next_seg:
	lw	r3, r1 ; AB
	shc	r3, 4
	nr	r3, 0xf000
	aw	r3, r2 ; NB

	lw	r4, r1 ; RAL
	shc	r4, 12
	nr	r4, 0b0000000011110000
	aw	r4, r2 ; MP
	slz	r4

	ou	r3, r4+mega+phide+done
	.word	err, err, ok, err
err:
	hlt	041
ok:
	; next segment/block
	awt	r1, 1
	cw	r1, 16
	jn	next_seg
	lwt	r1, 0
	awt	r2, 1
	cw	r2, 16
	jn	next_seg

	uj	[mem_cfg]

