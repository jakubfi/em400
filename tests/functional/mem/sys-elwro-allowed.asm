; Elwro allows writing logical addresses 0:0 and 0:1 to RAL registers
; of physical frames other than 0:0 and 0:1 (reserved for OS)
; In other words: it's allowed to map logical addresses 0:0 and 0:1
; to physical frames other than 0:0 and 0:1

	.include cpu.inc
	.include io.inc

	uj	start

mask:	.word	IMASK_NOMEM

nomem_proc:
	awt	r7, 1
	hlt	041

	.org	OS_START

start:	lwt	r7, 0
	lw	r1, stack
	rw	r1, STACKP
	lwt	r1, nomem_proc
	rw	r1, INTV_NOMEM
	im	mask

	lwt	r1, 0
	ou	r1, 5\10 | 1\14 | MEM_CFG
	.word	err0, err0, next, err0
err0:	hlt	042

next:	lw	r1, 1\3
	ou	r1, 5\10 | 1\14 | MEM_CFG
	.word	err1, err1, ok, err1

err1:	hlt	043
ok:	hlt	077
stack:

; XPCT r7 : 0
; XPCT ir : 0xec3f
