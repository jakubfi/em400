.program "cpu/r0"

; PRE r0 = 0xaa00

	lw r0, 0xfffe

	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT hex(r0): 0xaafe

