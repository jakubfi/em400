.prog "mem/elwro-realloc"

; Elwro does not allow to map the same segment to two different
; addresses. First mapping is removed, when second one is done

	.equ int_nomem 0x40 + 2
	.equ stackp 0x61
	.equ magic 0x2323
	.equ seg1 2\3
	.equ seg2 3\3
	.equ addr 100

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

	lw r1, seg1 + 0
	ou r1, 2\10 + 0\14 + 1
	.data err, err, ok, err

ok:	im mask
	lw r1, magic
	rw r1, seg1 + addr

	lw r1, seg2 + 0
	ou r1, 2\10 + 0\14 + 1
	.data err, err, ok2, err

ok2:	lw r1, [seg1+addr]
	lw r2, [seg2+addr]

	hlt 077

.finprog

; XPCT hex(r2) : 0x2323
; XPCT int(r7) : 1
; XPCT oct(ir[10-15]) : 077
