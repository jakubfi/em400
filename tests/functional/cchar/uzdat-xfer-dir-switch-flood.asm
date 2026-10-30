; CONFIG configs/uzdat.ini
; PRECMD CLOCK ON

; Repeated writes refused while UZDAT switches transmission to OUT must not
; postpone the switch: flooding writes eventually gets one accepted.
; Channel interrupts stay masked, READY vs. "nieaktualne" depends on timing.

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
mask_none:
	.word	0

; ------------------------------------------------------------------------
int_timer:
	awt	r7, -1		; one timer interrupt waited
	jz	.timeout	; switch to OUT never happened
	lip
.timeout:
	hlt	077
	ujs	.timeout

; ------------------------------------------------------------------------
; ------------------------------------------------------------------------
; ------------------------------------------------------------------------

start:
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, int_timer
	rw	r1, INTV_TIMER

	lwt	r7, 5		; timer interrupts to wait for the 2 ms direction switch
	lwt	r6, 0		; write accepted flag

	im	mask_timer

.loop:
	ou	r3, KZ_CMD_DEV_WRITE | UZDAT
	.word	.no1, .en1, .ok1, .pe1
.no1:
	hlt	040
	ujs	.no1
.pe1:
	hlt	042
	ujs	.pe1
.en1:
	ujs	.loop

.ok1:
	lwt	r6, 1
	im	mask_none
.done:
	hlt	077
	ujs	.done

stack:

; XPCT r6 : 1
