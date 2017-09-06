
	.include hw.inc

	uj	start

stack:	.res	4
softx:	lip

mask:	.word	IMASK_SOFT
one:	.word	1
minus1:	.word	-1

	.org	OS_MEM_BEG

start:	lwt	r1, stack
	rw	r1, STACKP
	lwt	r1, softx
	rw	r1, IV_SW_HIGH
	im	mask
	lwt	r1, 0

loop:	lwt	r2, -1		; short arg
	md	[minus1]	; d-mod, pre-mod
	aw	r3, r2 + 3	; b-mod, add, M-arg
	ujs	0		; empty cycle
	siu			; int serve
	shc	r3, 5		; shift
	or	r3, 0xbaba	; binary arithmetic
	cw	r3, [one]	; comparison
	lb	r3, one		; byte addressing
	tw	r3, one		; mem get
	pw	r3, one		; mem put
	bb	r3, 1		; P
	ujs	0
	rj	r7, jump

	irb	r1, loop
	ujs	loop
	hlt	077

jump:	uj	r7

