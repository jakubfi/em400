; Mapping a non-existent module fails

	.include cpu.inc
	.include io.inc

	uj	start

	.org	OS_START

mask:	.word	IMASK_NOMEM

nomem_proc:
	hlt	040

start:
	; initialize interrupts
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, nomem_proc
	rw	r1, INTV_NOMEM
	im	mask

	; try to map 0:2 -> 3:0
	lw	r1, 0\MEM_SEGMENT | 2\MEM_PAGE
	ou	r1, 2\MEM_MODULE | 0\MEM_FRAME | MEM_CFG
	.word	.no, .en, .ok, .pe
.no:	hlt	077
.en:	hlt	041
.ok:	hlt	042
.pe:	hlt	043

stack:

; XPCT ir : 0xec3f
