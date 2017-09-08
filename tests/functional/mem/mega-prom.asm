; OPTS -c configs/mega_max.cfg

; does PROM show/hide work?
; is data preserved between PROM hide/shows?
; is write to PROM ineffective?
; does PROM allocate only last segment of block 0?

	.include hw.inc
	.include io.inc
	.include mega.inc

	.const	magic 0x2323
	.const	nb 0
	.const	ab 15\3
	.const	mp 0\14
	.const	seg 2\10

	uj	start

mask:	.word	IMASK_NOMEM

nomem_proc:
	aw	r7, 1
	lip

err:	hlt	040

	.org	OS_MEM_BEG

start:	lwt	r7, 0
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, nomem_proc
	rw	r1, IV_NOMEM

	lw	r1, 14\3 | nb
	ou	r1, mp | 3\10 | MEGA_ALLOC | MEM_CFG
	.word	err, err, ok0, err

ok0:	lw	r1, ab | nb
	ou	r1, mp | seg | MEGA_ALLOC | MEGA_PAS_HIDE | MEGA_ALLOC_FINISH | MEM_CFG
	.word	err, err, ok, err

ok:	im	mask
	lw	r1, magic
	rw	r1, ab

	lw	r1, 0
	ou	r1, MEGA_ALLOC + MEGA_PAS_SHOW + MEM_CFG
	.word	err, err, ok2, err

ok2:	lw	r2, magic
	rw	r2, ab+1
	rw	r2, ab-1
	lw	r2, [ab+1]
	lw	r3, [ab-1]
	lw	r4, [ab]

	lw	r1, 0
	ou	r1, MEGA_ALLOC | MEGA_PAS_HIDE | MEM_CFG
	.word	err, err, ok3, err

ok3:	lw	r5, [ab]

	hlt	077
stack:

; XPCT r2 : 0x0000
; XPCT r3 : 0x2323
; XPCT r4 : 0x0000
; XPCT r5 : 0x2323
; XPCT r7 : 0
; XPCT ir : 0xec3f
