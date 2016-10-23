
	.const	int_nomem 0x40 + 2
	.const	stackp 0x61

	lwt	r1, 50
	lwt	r2, 60

	lw	r3, stack
	rw	r3, stackp
	lw	r3, nomem_proc
	rw	r3, int_nomem

	lw	r3, 0b0000000000000001
	ou	r3, 0b0000000000000011
	.word	err, err, ok, err
ok:
	mb	blk
	im	blk

	pf	220

	lwt	r1, 0
	lwt	r2, 0

	tf	220

	hlt	077

err:	hlt	040
blk:	.word	0b0100000000000001

nomem_proc:
	hlt	040
stack:

; XPCT rz[6] : 0
; XPCT sr : 0b0100000000000001

; XPCT r1 : 50
; XPCT r2 : 60
; XPCT ir&0x3f : 0o77
