
	.include cpu.inc

	lw	r5, 50
	lw	r6, 60
	lw	r7, 70

	lw	r3, stack
	rw	r3, STACKP
	lw	r3, nomem_proc
	rw	r3, INTV_NOMEM

	lw	r1, 0b0000000000000001
	ou	r1, 0b0000000000000011
	.word	err, err, ok, err
ok:
	mb	blk
	im	blk

	pa	120

	lwt	r5, 0
	lwt	r6, 0
	lwt	r7, 0

	ta	120

	hlt	077

err:	hlt	040
blk:	.word	IMASK_NOMEM | 1

nomem_proc:
	hlt	040
stack:

; XPCT rz[6] : 0
; XPCT sr : 0b0100000000000001

; XPCT r1 : 1
; XPCT r5 : 50
; XPCT r6 : 60
; XPCT r7 : 70
; XPCT ir : 0xec3f
