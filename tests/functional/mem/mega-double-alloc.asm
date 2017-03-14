; OPTS -c configs/mega_max.cfg

; does MEGA allow to allocate the same physical segment for two logical segments?

	.equ int_nomem 0x40 + 2
	.equ stackp 0x61
	.equ magic 0x2323

	.equ nb 0
	.equ ab1 2\3
	.equ ab2 3\3
	.equ mp 0\14
	.equ seg 2\10
	.equ addr 100

	.equ mega 1\15 + 1\6
	.equ dealloc 1\5
	.equ done 1\0
	.equ pshow 1\2
	.equ phide 1\1

	uj start

mask:	.word 0b0100000000000000
stack:	.res 16

nomem_proc:
	aw r7, 1
	lip
err:	hlt 040

	.org 0x70

start:	lwt r7, 0
	lw r1, stack
	rw r1, stackp
	lw r1, nomem_proc
	rw r1, int_nomem

	lw r1, ab1 + nb
	ou r1, seg + mp + mega+phide+done
	.word err, err, ok, err

ok:	lw r1, ab2 + nb
	ou r1, seg + mp + mega
	.word err, err, ok2, err

ok2:	im mask
	lw r1, magic
	rw r1, ab1 + addr
	lw r2, [ab1+addr]
	lw r3, [ab2+addr]

	hlt 077

; XPCT r2 : 0x2323
; XPCT r3 : 0x2323
; XPCT r7 : 0
; XPCT ir : 0xec3f
