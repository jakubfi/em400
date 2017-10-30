; Mapping pages >1 in segment 0

	.include hw.inc
	.include io.inc

	.const	ADDR 0x3333
	.const	MAGIC 0xbaca

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

	lw	r1, 3\3 | 0\15
	ou	r1, 3\10 | 0\14 | MEM_CFG
	.word	n1, e1, o1, p1
n1:	hlt	050
e1:	hlt	051
p1:	hlt	052
o1:
	lw	r1, MAGIC
	rw	r1, ADDR

	lw	r1, 3\3 | 0\15
	ou	r1, 11\10 | 0\14 | MEM_CFG
	.word	n2, e2, o2, p2
n2:	hlt	050
e2:	hlt	051
p2:	hlt	052
o2:
	lw	r2, [ADDR]
	cw	r2, MAGIC
	jes	ok
	hlt	055
ok:	hlt	077
stack:

; XPCT r7 : 0
; XPCT ir : 0xec3f
