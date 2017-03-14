; does Elwro allocation work?
; can we read from/write to allocated segments?

	.equ int_nomem 0x40 + 2
	.equ stackp 0x61
	.equ magic 0x2323
	.equ addr 100
	.equ ab_s 1\3
	.equ seg_s 1\10

	uj start

mask:	.word 0b0100000000000000
stack:	.res 16

nomem_proc:
	awt r7, 1
err:	hlt 040

	.org 0x70

start:	lwt r7, 0
	lwt r1, stack
	rw r1, stackp
	lwt r1, nomem_proc
	rw r1, int_nomem

	lw r1, ab_s
	lw r2, seg_s

next:	cw r1, 7\3
	jes fin
	aw r1, ab_s
	aw r2, seg_s
	ou r1, r2+1
	.word err, err, next, err

fin:	im mask
	lwt r1, 0
loop:	aw r1, 0x1000
	rw r1, r1
	lw r1, [r1]
	rw r1, r1+0x100
	cw r1, 0x7000
	jn loop

	hlt 077

; XPCT r7 : 0
; XPCT [0x1100] : 0x1000
; XPCT [0x2100] : 0x2000
; XPCT [0x3100] : 0x3000
; XPCT [0x4100] : 0x4000
; XPCT [0x5100] : 0x5000
; XPCT [0x6100] : 0x6000
; XPCT [0x7100] : 0x7000
; XPCT ir : 0xec3f
