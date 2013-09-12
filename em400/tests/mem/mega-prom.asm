.prog "mem/mega-prom"

; CONFIG configs/mega_max.conf

; does PROM show/hide work?
; is data preserved between PROM hide/shows?
; is write to PROM ineffective?
; does PROM allocate only last segment of block 0?

	.equ int_nomem 0x40 + 2
	.equ stackp 0x61
	.equ magic 0x2323

	.equ nb 0
	.equ ab 15\3
	.equ mp 0\14
	.equ seg 2\10

	.equ mega 1\15 + 1\6
	.equ dealloc 1\5
	.equ done 1\0
	.equ pshow 1\2
	.equ phide 1\1

	uj start

mask:	.data 0b0100000000000000
stack:	.res 16

nomem_proc:
	aw r7, 1
	lip

err:	hlt 040

	.ic 0x70

start:	lw r1, stack
	rw r1, stackp
	lw r1, nomem_proc
	rw r1, int_nomem

	lw r1, 14\3 + nb
	ou r1, mp + 3\10 + mega
	.data err, err, ok0, err

ok0:	lw r1, ab + nb
	ou r1, mp + seg + mega+phide+done
	.data err, err, ok, err

ok:	im mask
	lw r1, magic
	rw r1, ab

	lw r1, 0
	ou r1, mega+pshow
	.data err, err, ok2, err

ok2:	lw r2, magic
	rw r2, ab+1
	rw r2, ab-1
	lw r2, [ab+1]
	lw r3, [ab-1]
	lw r4, [ab]

	lw r1, 0
	ou r1, mega+phide
	.data err, err, ok3, err

ok3:	lw r5, [ab]

	hlt 077

.finprog

; XPCT hex(r2) : 0x0000
; XPCT hex(r3) : 0x2323
; XPCT hex(r4) : 0x0000
; XPCT hex(r5) : 0x2323
; XPCT int(r7) : 0
; XPCT oct(ir[10-15]) : 077
