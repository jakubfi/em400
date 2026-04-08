; OPTS -c configs/uzdat.ini
; PRECMD CLOCK ON

; first read from UZDAT should switch transmission to OUT after a while

	.cpu	mera400

	.include cpu.inc
	.include io.inc

	mcl
	uj	start

	.const	CHAR_CHAN 7
	.const	UZDAT_DEV 0
	.const	UZDAT CHAR_CHAN\IO_CHAN | UZDAT_DEV\IO_DEV
	.org	OS_START

; ------------------------------------------------------------------------
mask:	.word	IMASK_GROUP_H | IMASK_CH4_9

int_uzdat:
	lwt	r6, 1	; uzdat interrupt marker
	lip
int_timer:
	cwt	r6, 1	; uzdat sent interrupt?
	jes	.fin	; yes
	awt	r7, 1	; wait more
	cwt	r7, 2	; done waiting?
	jes	.fail	; yes
	lip		; no
.fail:	hlt	060
	ujs	.fail
.fin:	hlt	077
	ujs	.fin

; ------------------------------------------------------------------------
; ------------------------------------------------------------------------
; ------------------------------------------------------------------------

start:
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, int_uzdat
	rw	r1, INTV_CH0 + CHAR_CHAN
	lw	r1, int_timer
	rw	r1, INTV_TIMER

	lwt	r7, 0
	lwt	r6, 0

	im	mask
.loop:
	ou	r3, KZ_CMD_DEV_WRITE | UZDAT
	.word	.no1, .en1, .ok1, .pe1
.no1:	hlt	040
	ujs	.no1
.ok1:	hlt	041
	ujs	.ok1
.pe1:	hlt	042
	ujs	.pe1
	; busy wait for the interrupt
.en1:	ujs	.en1

stack:

; XPCT ir : 0xec3f
; XPCT r6 : 1
; XPCT alarm : 0
