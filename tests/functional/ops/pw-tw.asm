
	.include cpu.inc
	.include io.inc

	lw	r0, 0xaafe
	lw	r1, 0x0001
	lw	r2, 0x0010
	lw	r3, 0x0100
	lw	r4, 0x1000
	lw	r5, 0xf0f0
	lw	r6, 0x0f0f
	lw	r7, 0x1234

	uj	start

blk:	.word	IMASK_NOMEM | 1
data:	.res	8
stack:	.res	16

nomem_proc:
	hlt	040

	.org	0x70
start:
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, nomem_proc
	rw	r1, INTV_NOMEM

	lw	r1, 0\3 | 1\15
	ou	r1, 0\10 | 1\14 | MEM_CFG
	.word	err, err, ok, err
err:	hlt	040
ok:
	mb	blk
	im	blk

	pw	r0, data+0
	pw	r1, data+1
	pw	r2, data+2
	pw	r3, data+3
	pw	r4, data+4
	pw	r5, data+5
	pw	r6, data+6
	pw	r7, data+7

	lwt	r1, 0
	lpc	r1
	lwt	r2, 0
	lwt	r3, 0
	lwt	r4, 0
	lwt	r5, 0
	lwt	r6, 0
	lwt	r7, 0

	tw	r0, data+0
	tw	r1, data+1
	tw	r2, data+2
	tw	r3, data+3
	tw	r4, data+4
	tw	r5, data+5
	tw	r6, data+6
	tw	r7, data+7

	hlt	077

; XPCT rz[6] : 0
; XPCT sr : 0b0100000000000001

; XPCT r0 : 0xaafe
; XPCT r1 : 0x0001
; XPCT r2 : 0x0010
; XPCT r3 : 0x0100
; XPCT r4 : 0x1000
; XPCT r5 : 0xf0f0
; XPCT r6 : 0x0f0f
; XPCT r7 : 0x1234
; XPCT ir : 0xec3f
