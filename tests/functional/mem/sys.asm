; Can't allocate hardwired segments

	.include hw.inc
	.include io.inc

	; TODO: test disabled, don't know why
	hlt	077

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

	lwt	r1, 0
	ou	r1, 5\10 + 1\14 + MEM_CFG
	.word	err0, err0, next, err0
err0:	hlt	042

next:	lw	r1, 1\3
	ou	r1, 5\10 + 1\14 + MEM_CFG
	.word	err1, err1, ok, err1

err1:	hlt	043
ok:	hlt	077
stack:

; XPCT r7 : 0
; XPCT ir : 0xec3f
