; Can't allocate hardwired segments

	.equ int_nomem 0x40 + 2
	.equ stackp 0x61
hlt 077
	uj start

mask:	.word 0b0100000000000000
stack:	.res 16

nomem_proc:
	awt r7, 1
	hlt 041

	.org 0x70

start:	lwt r7, 0
	lwt r1, stack
	rw r1, stackp
	lwt r1, nomem_proc
	rw r1, int_nomem
	im mask

	lwt r1, 0
	ou r1, 5\10 + 1\14 + 1
	.word err0, err0, next, err0
err0:	hlt 042

next:	lw r1, 1\3
	ou r1, 5\10 + 1\14 + 1
	.word err1, err1, ok, err1

err1:	hlt 043
ok:	hlt 077

; XPCT r7 : 0
; XPCT ir : 0xec3f
