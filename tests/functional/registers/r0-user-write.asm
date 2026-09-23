; in user mode explicit register writes to r0 change only right byte

; PRECMD REG KB 0x1234

	.include cpu.inc

	.const	USER_IMASK IMASK_PARITY | IMASK_NOMEM | IMASK_CPU_H | IMASK_IFPOWER | IMASK_GROUP_H

	uj	start

exl_handler:
	hlt	077
illegal_handler:
	hlt	040

user_vec:
	.word	user_prog, 0, USER_IMASK | 1\SR_Q

	.org	OS_START
start:
	lwt	r1, exl_handler
	rw	r1, EXLV
	lwt	r1, illegal_handler
	rw	r1, INTV_ILLEGAL
	rw	r1, INTV_NOMEM
	lw	r1, stack
	rw	r1, STACKP
	sp	user_vec

data:	.word	0x1234

user_prog:
	lwt	r5, 0
	lw	r6, 0x5a3c
	lw	r4, 0x02ff
	lw	r3, 0x0200

	lpc	r6
	lw	r0, 0x1234
	cw	r0, 0x5a34
	jn	fail
	awt	r5, 1

	lpc	r6
	lwt	r0, -1
	cw	r0, 0x5aff
	jn	fail
	awt	r5, 1

	lpc	r6
	tw	r0, data
	cw	r0, 0x5a34
	jn	fail
	awt	r5, 1

	lpc	r6
	lws	r0, data
	cw	r0, 0x5a34
	jn	fail
	awt	r5, 1

	lw	r7, 0x0f0f
	lpc	r6
	ls	r0, 0xffff
	cw	r0, 0x5a3f
	jn	fail
	awt	r5, 1

	lw	r1, rj_ret & 0xff | 0x5a00
	lpc	r6
	rj	r0, rj_ret
rj_ret:
	cw	r0, r1
	jn	fail
	awt	r5, 1

	lw	r1, ric_ret & 0xff | 0x5a00
	lpc	r6
	ric	r0
ric_ret:
	cw	r0, r1
	jn	fail
	awt	r5, 1

	lpc	r6
	rky	r0
	cw	r0, 0x5a34
	jn	fail
	awt	r5, 1

	lpc	r6
	zlb	r0
	cw	r0, 0x5a3c
	jn	fail
	awt	r5, 1

	lpc	r6
	shc	r0, 8
	cw	r0, 0x5a5a
	jn	fail
	awt	r5, 1

	lpc	r4
	ri	r0, 0x1111
	cw	r0, 0x0200
	jn	fail
	lw	r1, [0x02ff]
	cw	r1, 0x1111
	jn	fail
	awt	r5, 1

	lpc	r4
	trb	r0, 1
	ujs	trb_ok
	uj	fail
trb_ok:
	cw	r0, 0x0200
	jn	fail
	awt	r5, 1

	lpc	r4
	irb	r0, irb_ok
	uj	fail
irb_ok:
	cw	r0, 0x0200
	jn	fail
	awt	r5, 1

	lpc	r3
	drb	r0, drb_ok
	uj	fail
drb_ok:
	cw	r0, 0x02ff
	jn	fail
	awt	r5, 1

	lpc	r6
	or	r0, 0x0100
	cw	r0, 0x5a3c
	jn	fail
	awt	r5, 1

	lpc	r6
	nr	r0, 0x00ff
	cw	r0, 0x5a3c
	jn	fail
	awt	r5, 1

	lpc	r6
	er	r0, 0xff00
	cw	r0, 0x5a3c
	jn	fail
	awt	r5, 1

	lpc	r6
	xr	r0, 0x5a3c
	cw	r0, 0xda00
	jn	fail
	awt	r5, 1

	lpc	r6
	aw	r0, 0x0100
	cw	r0, 0x0a3c
	jn	fail
	awt	r5, 1

	lpc	r6
	sw	r0, 0x0100
	cw	r0, 0x1a3c
	jn	fail
	awt	r5, 1

	lpc	r6
	slz	r0
	cw	r0, 0x5a78
	jn	fail
	awt	r5, 1

	lpc	r6
	svz	r0
	cw	r0, 0x7a78
	jn	fail
	awt	r5, 1

	lpc	r6
	srz	r0
	cw	r0, 0x5a1e
	jn	fail
	awt	r5, 1

	lpc	r6
	ngl	r0
	cw	r0, 0x5ac3
	jn	fail
	awt	r5, 1

	exl	0
fail:
	hlt	040

stack:

; XPCT rz[6] : 0
; XPCT ir : 0xec3f
; XPCT r5 : 24
