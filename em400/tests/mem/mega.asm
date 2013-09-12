.prog "mem/MEGA"

; CONFIG configs/mega_max.conf

; does MEGA allocation work?
; can we read from/write to allocated segments?

	.equ int_nomem 0x40 + 2
	.equ stackp 0x61
	.equ magic 0x2323

	.equ nb 0
	.equ ab_s 1\3
	.equ ab_last 15\3
	.equ mp 0
	.equ seg_s 1\10
	.equ addr 100

	.equ mega 1\15 + 1\6
	.equ dealloc 1\5
	.equ done 1\0
	.equ pshow 1\2
	.equ phide 1\1

	uj start

mask:	.data 0b0100000000000000
stack:	.res 16

nomem_proc:
	awt r7, 1
err:	hlt 040

	.ic 0x70

start:	lwt r1, stack
	rw r1, stackp
	lwt r1, nomem_proc
	rw r1, int_nomem

	lw r1, ab_s + nb
	lw r2, seg_s + mp

next:	cw r1, ab_last
	jes fin
	aw r1, ab_s
	aw r2, seg_s
	ou r1, r2 + mega+phide+done
	.data err, err, next, err

fin:	im mask
	lwt r1, 0
loop:	aw r1, 0x1000
	rw r1, r1
	lw r1, [r1]
	rw r1, r1+0x100
	cw r1, 0xf000
	jn loop

	hlt 077

.finprog

; XPCT int(r7) : 0
; XPCT hex([0x1100]) : 0x1000
; XPCT hex([0x2100]) : 0x2000
; XPCT hex([0x3100]) : 0x3000
; XPCT hex([0x4100]) : 0x4000
; XPCT hex([0x5100]) : 0x5000
; XPCT hex([0x6100]) : 0x6000
; XPCT hex([0x7100]) : 0x7000
; XPCT hex([0x8100]) : 0x8000
; XPCT hex([0x9100]) : 0x9000
; XPCT hex([0xa100]) : 0xa000
; XPCT hex([0xb100]) : 0xb000
; XPCT hex([0xc100]) : 0xc000
; XPCT hex([0xd100]) : 0xd000
; XPCT hex([0xe100]) : 0xe000
; XPCT hex([0xf100]) : 0xf000
; XPCT oct(ir[10-15]) : 077
