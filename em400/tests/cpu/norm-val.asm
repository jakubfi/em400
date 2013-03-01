.program "cpu/norm-val"

	lw r1, 0x0001
	lw r2, 0x0002
	lw r3, r2
	lw r4, r2 + 0x00f0
	lw r5, r1 + r4
	lw r6, r1 - 1
	lw r7, r1 - 2

	hlt 077

.endprog

; XPCT hex(r1): 0x0001
; XPCT hex(r2): 0x0002
; XPCT hex(r3): 0x0002
; XPCT hex(r4): 0x00f2
; XPCT hex(r5): 0x00f3
; XPCT hex(r6): 0x0000
; XPCT hex(r7): 0xffff

# vim: tabstop=4
