; CONFIG configs/mega_max.cfg

; does MEGA deallocation work?

	.equ int_nomem 0x40 + 2
	.equ stackp 0x61
	.equ magic 0x2323

	.equ nb 1\15
	.equ ab 2\3
	.equ mp 0\14
	.equ seg 2\10

	.equ mega 1\15 + 1\6
	.equ dealloc 1\5
	.equ done 1\0
	.equ pshow 1\2
	.equ phide 1\1

	uj start

mask:	.word 0b0100000000000001
stack:	.res 16

nomem_proc:
	aw r7, 1
	lip

err:	hlt 040

	.org 0x70

start:	lw r1, stack
	rw r1, stackp
	lw r1, nomem_proc
	rw r1, int_nomem

	lw r1, ab + nb
	ou r1, mp + seg + mega+phide+done
	.word err, err, ok, err

ok:	mb mask
	im mask
	lw r1, magic
	pw r1, ab

	lw r1, ab + nb
	ou r1, mega+dealloc
	.word err, err, ok2, err

ok2:	tw r1, ab

	lw r1, ab + nb
	ou r1, mp + seg + mega
	.word err, err, ok3, err

ok3:	tw r2, ab

	hlt 077

; XPCT hex(r2) : 0x2323
; XPCT int(r7) : 1
; XPCT oct(ir[10-15]) : 077
