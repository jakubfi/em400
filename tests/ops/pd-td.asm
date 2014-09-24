
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

; XPCT int(rz[6]) : 0
; XPCT bin(sr) : 0b0100000000000001

; XPCT int(r1): 50
; XPCT int(r2): 60
; XPCT oct(ir[10-15]) : 077
