; OPTS -c configs/mega_max.cfg

; does MEGA allocation work?
; can we read from/write to allocated segments?

	.include hw.inc
	.include io.inc
	.include mega.inc

	.const	magic 0x2323
	.const	nb 0
	.const	ab_s 1\3
	.const	ab_last 15\3
	.const	mp 0
	.const	seg_s 1\10
	.const	addr 100

	uj	start

mask:	.word	IMASK_NOMEM

nomem_proc:
	awt	r7, 1
err:	hlt	040

	.org	OS_MEM_BEG

start:	lwt	r7, 0
	lw	r1, stack
	rw	r1, STACKP
	lwt	r1, nomem_proc
	rw	r1, IV_NOMEM

	lw	r1, ab_s + nb
	lw	r2, seg_s + mp

next:	cw	r1, ab_last
	jes	fin
	aw	r1, ab_s
	aw	r2, seg_s
	ou	r1, r2 + MEGA_ALLOC | MEGA_PAS_HIDE | MEGA_ALLOC_FINISH | MEM_CFG
	.word	err, err, next, err

fin:	im	mask
	lwt	r1, 0
loop:	aw	r1, 0x1000
	rw	r1, r1
	lw	r1, [r1]
	rw	r1, r1 + 0x100
	cw	r1, 0xf000
	jn	loop

	hlt	077
stack:

; XPCT r7 : 0
; XPCT [0x1100] : 0x1000
; XPCT [0x2100] : 0x2000
; XPCT [0x3100] : 0x3000
; XPCT [0x4100] : 0x4000
; XPCT [0x5100] : 0x5000
; XPCT [0x6100] : 0x6000
; XPCT [0x7100] : 0x7000
; XPCT [0x8100] : 0x8000
; XPCT [0x9100] : 0x9000
; XPCT [0xa100] : 0xa000
; XPCT [0xb100] : 0xb000
; XPCT [0xc100] : 0xc000
; XPCT [0xd100] : 0xd000
; XPCT [0xe100] : 0xe000
; XPCT [0xf100] : 0xf000
; XPCT ir : 0xec3f
