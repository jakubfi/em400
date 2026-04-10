; OPTS -c configs/uzdat.ini
; PRECMD CLOCK ON

; First, single write to UZDAT should switch transmission to OUT
; after 2 ms. UZDAT then reports a "READY" interrupt.

	.cpu	mera400

	.include cpu.inc
	.include io.inc

	mcl
	uj	start

	.const	CHAR_CHAN 7
	.const	UZDAT_DEV 0
	.const	UZDAT CHAR_CHAN\IO_CHAN | UZDAT_DEV\IO_DEV
	.const	INTSPEC_READY 1\7
	.org	OS_START

mask:	.word	IMASK_GROUP_H | IMASK_CH4_9

; ------------------------------------------------------------------------
int_uzdat:
	md	[STACKP]
	lw	r1, [-SP_SPEC]
	cl	r1, INTSPEC_READY
	jn	.bad_spec
	awt	r6, 1		; UZDAT sent good interrupt
	lip
.bad_spec:
	lwt	r5, 1		; UZDAT sent wrong intspec
	lip

; ------------------------------------------------------------------------
int_timer:
	awt	r7, -1		; one timer interrupt waited
	jz	.done		; done waiting
	lip			; wait more
.done:
	hlt	077
	ujs	.done

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

	lwt	r7, 5		; wait this many timer interrupts for the UZDAT interrupt
	lwt	r6, 0		; number of interrupts reported by UZDAT
	lwt	r5, 0		; bad interrupt spec flag

	im	mask

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
; XPCT r5 : 0
; XPCT r7 : 0
; XPCT alarm : 0
