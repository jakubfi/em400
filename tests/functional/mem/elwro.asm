; does Elwro allocation work?
; can we read from/write to allocated segments?

	.include cpu.inc
	.include io.inc

	.const	magic 0x2323
	.const	addr 100
	.const	ab_s 1\3
	.const	seg_s 1\10

	uj	start

mask:	.word	IMASK_NOMEM

nomem_proc:
	awt	r7, 1
err:	hlt	040

	.org	OS_START

start:	lwt	r7, 0
	lw	r1, stack
	rw	r1, STACKP
	lwt	r1, nomem_proc
	rw	r1, INTV_NOMEM

	lw	r1, ab_s
	lw	r2, seg_s

next:	cw	r1, 7\3
	jes	fin
	aw	r1, ab_s
	aw	r2, seg_s
	ou	r1, r2 + MEM_CFG
	.word	err, err, next, err

fin:	im	mask
	lwt	r1, 0
loop:	aw	r1, 0x1000
	rw	r1, r1
	lw	r1, [r1]
	rw	r1, r1+0x100
	cw	r1, 0x7000
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
; XPCT ir : 0xec3f
