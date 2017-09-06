; OPTS -c configs/multix.cfg
; PRECMD CLOCK ON

; If reset is done before previous reset routine finished, no IWYZE should appear
; (except the final one)

	.include hw.inc
	.include mx.inc

	uj	start

mask0:	.word	IMASK_NONE
mask:	.word	IMASK_CPU | IMASK_CH0_1

	.org	OS_MEM_BEG

; ------------------------------------------------------------------------
tim_proc:
	cw	r1, 0
	jes	done
	awt	r1, -1
again:	in	r5, 0\2 + MX_CHAN	; reset MULTIX
	.word	fail, again, ok, fail
fail:	hlt	040
ok:
done:	lip

; ------------------------------------------------------------------------
mx_proc:
	im	mask0
	cw	r1, 0		; MULTIX interrupt should come after r1 counter drops to 0
	jn	too_early	; MULTIX interrupt came too early
	lw	r4, [STACKP]
	lw	r4, [r4-1]
	cw	r4, MX_IWYZE
	jn	int_fail
	hlt	077
int_fail:
	hlt	041
too_early:
	hlt	042

; ------------------------------------------------------------------------
start:
	lw	r3, stack
	rw	r3, STACKP
	lw	r3, mx_proc
	rw	r3, MX_IV
	lw	r3, tim_proc
	rw	r3, IV_TIMER
	rw	r3, IV_EXTRA
	lw	r1, 100		; reset counter: wait 100 timer interrupts (100*10ms = 1s)

	im	mask
loop:	hlt
	ujs	loop
	im	mask0
	hlt	050

stack:

; XPCT rz[15] : 0
; XPCT rz[6] : 0
; XPCT alarm : 0
; XPCT r1 : 0
; XPCT ir : 0xec3f
