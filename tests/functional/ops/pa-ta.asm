
	.include cpu.inc

	lw	r2, 20
	lw	r3, 30
	lw	r4, 40
	lw	r5, 50
	lw	r6, 60
	lw	r7, 70

	lw	r1, stack
	rw	r1, STACKP
	lw	r1, nomem_proc
	rw	r1, INTV_NOMEM

	lw	r1, 0b0000000000000001
	ou	r1, 0b0000000000000011
	.word	err, err, ok, err
ok:
	mb	blk
	im	blk

	pa	120

	lwt	r1, 0
	lwt	r2, 0
	lwt	r3, 0
	lwt	r4, 0
	lwt	r5, 0
	lwt	r6, 0
	lwt	r7, 0

	ta	120

	hlt	077

data:	.res	7
blk:	.word	IMASK_NOMEM | 1

nomem_proc:
	hlt	040
stack:

err:	hlt	040

; XPCT rz[6] : 0
; XPCT sr : 0b0100000000000001

; XPCT r1 : 1
; XPCT r2 : 20
; XPCT r3 : 30
; XPCT r4 : 40
; XPCT r5 : 50
; XPCT r6 : 60
; XPCT r7 : 70
; XPCT ir : 0xec3f
