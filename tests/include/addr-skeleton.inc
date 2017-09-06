
	.include hw.inc
	.include io.inc
	.include mega.inc

	uj	start

; ------------------------------------------------------------------------
mask:	.word	IMASK_NOMEM
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
	lwt	r1, 2 ; first page to configure
next_page:
	lw	r3, r1 ; page
	shc	r3, 4
	lw	r4, r1 ; frame
	shc	r4, 11
	nr	r4, 0b0000000011100000 ; take 3 leas sig. bits
	lw	r2, r1 ; module
	shc	r2, 2
	nr	r2, 0b0000000000000010 ; use msb to switch module to 1 for segments >8 (to allow 32K modules)
	aw	r4, r2
	ou	r3, r4 + MEGA_ALLOC | MEGA_PAS_HIDE | MEGA_ALLOC_FINISH | MEM_CFG
	.word	err, err, ok, err
err:	hlt	041
ok:	; next page
	awt	r1, 1
	cw	r1, 16
	jls	next_page
	uj	[mem_cfg]

; vim: tabstop=8 shiftwidth=8 autoindent syntax=emas
