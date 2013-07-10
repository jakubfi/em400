.prog "op/PW+TW"

; PRE r0 = 0xaafe
; PRE r1 = 0x0001
; PRE r2 = 0x0010
; PRE r3 = 0x0100
; PRE r4 = 0x1000
; PRE r5 = 0xf0f0
; PRE r6 = 0x0f0f
; PRE r7 = 0x1234

	lw r1, 0b0000000000000001
	ou r1, 0b0000000000000011
	.data   err, err, ok, err
ok:
	mb blk

	pw r0, 20
	pw r1, 21
	pw r2, 22
	pw r3, 23
	pw r4, 24
	pw r5, 25
	pw r6, 26
	pw r7, 27

	lwt r1, 0
	lpc r1
	lwt r2, 0
	lwt r3, 0
	lwt r4, 0
	lwt r5, 0
	lwt r6, 0
	lwt r7, 0

	tw r0, 20
	tw r1, 21
	tw r2, 22
	tw r3, 23
	tw r4, 24
	tw r5, 25
	tw r6, 26
	tw r7, 27

	hlt 077

blk:	.data 0b0000000000000001

err:	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 1

; XPCT hex(r0): 0xaafe
; XPCT hex(r1): 0x0001
; XPCT hex(r2): 0x0010
; XPCT hex(r3): 0x0100
; XPCT hex(r4): 0x1000
; XPCT hex(r5): 0xf0f0
; XPCT hex(r6): 0x0f0f
; XPCT hex(r7): 0x1234
; XPCT int(ic): 51
