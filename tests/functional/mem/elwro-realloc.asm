; does Elwro prohibit allocating the same physical segment for two logical segments?
; is data in physical segment preserved between allocations?

	.equ int_nomem 0x40 + 2
	.equ stackp 0x61
	.equ magic 0x2323
	.equ nb 1\15
	.equ ab1 2\3
	.equ ab2 3\3
	.equ mp 0\14
	.equ seg 2\10
	.equ addr 100

	uj start

mask:	.word 0b0100000000000001
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
	ou r1, mp + seg + 1\15
	.word err, err, ok, err

ok:	mb mask
	im mask
	lw r1, magic
	pw r1, ab1 + addr

	lw r1, ab2 + nb
	ou r1, mp + seg + 1\15
	.word err, err, ok2, err

ok2:	tw r1, ab1+addr
	tw r2, ab2+addr

	hlt 077

; XPCT r2 : 0x2323
; XPCT r7 : 1
; XPCT ir : 0xec3f
