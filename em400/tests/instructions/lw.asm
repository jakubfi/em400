.program "instruction/LW"

	lw r0, 0xfffe
	lw r1, 0x0001
	lw r2, 0x0010
	lw r3, 0x0100
	lw r4, 0x1000
	lw r5, 0xf0f0
	lw r6, 0x0f0f
	lw r7, 0x1234

	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT hex(r0): 0x00fe
; XPCT hex(r1): 0x0001
; XPCT hex(r2): 0x0010
; XPCT hex(r3): 0x0100
; XPCT hex(r4): 0x1000
; XPCT hex(r5): 0xf0f0
; XPCT hex(r6): 0x0f0f
; XPCT hex(r7): 0x1234

