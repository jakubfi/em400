; CONFIG configs/uzdat.ini
; PRECMD CLOCK ON

; A refused write makes UZDAT switch to OUT and raise READY. While READY is
; still pending (channel interrupts masked), a write is accepted: READY is
; out of date and the interrupt arrives with spec 0 ("nieaktualne").

	.cpu	mera400

	.include cpu.inc
	.include io.inc

	mcl
	uj	start

	.const	CHAR_CHAN 7
	.const	UZDAT_DEV 0
	.const	UZDAT CHAR_CHAN\IO_CHAN | UZDAT_DEV\IO_DEV
	.org	OS_START

mask_timer:
	.word	IMASK_GROUP_H
mask_all:
	.word	IMASK_GROUP_H | IMASK_CH4_9
mask_none:
	.word	0

; ------------------------------------------------------------------------
int_uzdat:
	md	[STACKP]
	lw	r5, [-SP_SPEC]
	awt	r6, 1
	lip

; ------------------------------------------------------------------------
int_timer:
	awt	r7, -1
	lip

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

	lwt	r6, 0		; number of interrupts reported by UZDAT
	lwt	r5, -1		; last interrupt spec
	lwt	r7, 3		; timer interrupts to wait for READY (2 ms direction switch)

	im	mask_timer

	ou	r3, KZ_CMD_DEV_WRITE | UZDAT
	.word	.no1, .en1, .ok1, .pe1
.no1:
	hlt	040
	ujs	.no1
.ok1:
	hlt	041
	ujs	.ok1
.pe1:
	hlt	042
	ujs	.pe1

.en1:
	cwt	r7, 0
	jn	.en1

	ou	r3, KZ_CMD_DEV_WRITE | UZDAT
	.word	.no2, .en2, .ok2, .pe2
.no2:
	hlt	043
	ujs	.no2
.en2:
	hlt	044
	ujs	.en2
.pe2:
	hlt	045
	ujs	.pe2

.ok2:
	lwt	r7, 3		; let the pending interrupt in, then wait for the write to finish
	im	mask_all
.wait:
	cwt	r7, 0
	jn	.wait
	im	mask_none
.done:
	hlt	077
	ujs	.done

stack:

; XPCT r6 : 1
; XPCT r5 : 0
