; OPTS -c configs/mega_max.ini

; does MEGA allocation work?
; can we read from/write to allocated pages?

	.include cpu.inc
	.include io.inc
	.include mega.inc

	.const	SEGMENT 0\MEM_SEGMENT
	.const	PAGE_S 1\MEM_PAGE
	.const	PAGE_LAST 15\MEM_PAGE
	.const	MODULE 0\MEM_MODULE
	.const	FRAME 1\MEM_FRAME

	uj	start

mask:	.word	IMASK_NOMEM

nomem_proc:
	awt	r7, 1
err:	hlt	040

	.org	OS_START

start:	lwt	r7, 0
	; initialize interrupt system
	lw	r1, stack
	rw	r1, STACKP
	lwt	r1, nomem_proc
	rw	r1, INTV_NOMEM
	im	mask

	; allocate all pages
	lw	r1, PAGE_S + SEGMENT
	lw	r2, FRAME + MODULE
next:	cw	r1, PAGE_LAST
	jes	fin
	aw	r1, PAGE_S
	aw	r2, FRAME
	ou	r1, r2 + MEGA_ALLOC | MEGA_EPROM_HIDE | MEGA_ALLOC_DONE | MEM_CFG
	.word	err, err, next, err

fin:	; write a word to each allocated page.
	; this should work with no "nomem" interrupt raised
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
