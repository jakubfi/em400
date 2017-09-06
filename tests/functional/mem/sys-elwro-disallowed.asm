; Elwro prohibits writing to RAL register of physical frames 0:0 and 0:1 (reserved for OS)
; In other words: it's disallowed to map any logical addres to physical frames 0:0 or 0:1

	.include hw.inc
	.include io.inc

	uj	start

mask:	.word	IMASK_NOMEM

nomem_proc:
	awt	r7, 1
	hlt	041

	.org	OS_MEM_BEG

start:	lwt	r7, 0
	lw	r1, stack
	rw	r1, STACKP
	lwt	r1, nomem_proc
	rw	r1, IV_NOMEM
	im	mask

	lw	r1, 2\3 + 2\15
	ou	r1, 0\10 + 0\14 + MEM_CFG
	.word	next, err0, err0, err0
err0:	hlt	042

next:	lw	r1, 2\3 + 2\15
	ou	r1, 1\10 + 0\14 + MEM_CFG
	.word	ok, err1, err1, err1

err1:	hlt	043
ok:	hlt	077
stack:

; XPCT r7 : 0
; XPCT ir : 0xec3f
