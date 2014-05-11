; PRE r0 = 0xaafe
; PRE r1 = 0x0001
; PRE r2 = 0x0010
; PRE r3 = 0x0100
; PRE r4 = 0x1000
; PRE r5 = 0xf0f0
; PRE r6 = 0x0f0f
; PRE r7 = 0x1234

	.equ int_nomem 0x40 + 2
	.equ stackp 0x61

	uj start

blk:	.word 0b0100000000000001
data:	.res 8
stack:	.res 16

nomem_proc:
	hlt 040

	.org 0x70
start:
	lw r1, stack
	rw r1, stackp
	lw r1, nomem_proc
	rw r1, int_nomem

	lw r1, 0b0000000000000001
	ou r1, 0b0000000000000011
	.word   err, err, ok, err
err:	hlt 040
ok:
	mb blk
	im blk

	pw r0, data+0
	pw r1, data+1
	pw r2, data+2
	pw r3, data+3
	pw r4, data+4
	pw r5, data+5
	pw r6, data+6
	pw r7, data+7

	lwt r1, 0
	lpc r1
	lwt r2, 0
	lwt r3, 0
	lwt r4, 0
	lwt r5, 0
	lwt r6, 0
	lwt r7, 0

	tw r0, data+0
	tw r1, data+1
	tw r2, data+2
	tw r3, data+3
	tw r4, data+4
	tw r5, data+5
	tw r6, data+6
	tw r7, data+7

	hlt 077

; XPCT int(rz[6]) : 0
; XPCT bin(sr) : 0b0100000000000001

; XPCT hex(r0): 0xaafe
; XPCT hex(r1): 0x0001
; XPCT hex(r2): 0x0010
; XPCT hex(r3): 0x0100
; XPCT hex(r4): 0x1000
; XPCT hex(r5): 0xf0f0
; XPCT hex(r6): 0x0f0f
; XPCT hex(r7): 0x1234
; XPCT oct(ir[10-15]) : 077
