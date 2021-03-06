
	.include cpu.inc

	lw	r1, 50
	lw	r2, 60
	lw	r3, 70
	lw	r4, stack
	rw	r4, STACKP
	lw	r4, nomem_proc
	rw	r4, INTV_NOMEM

	lw	r4, 0b0000000000000001
	ou	r4, 0b0000000000000011
	.word	err, err, ok, err
ok:
	mb	blk
	im	blk

	pf	220

	lwt	r1, 0
	lwt	r2, 0
	lwt	r3, 0

	tf	220

	hlt	077

blk:	.word	IMASK_NOMEM | 1

err:	hlt	040

nomem_proc:
	hlt	040
stack:

; XPCT rz[6] : 0
; XPCT sr : 0b0100000000000001

; XPCT r1 : 50
; XPCT r2 : 60
; XPCT r3 : 70
; XPCT ir : 0xec3f
