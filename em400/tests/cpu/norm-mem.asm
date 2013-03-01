.program "cpu/norm-mem"

	lw r1, [dt1]
	lw r2, [dt2]
	lw r3, [r2]
	lw r4, [dt1 + 1]
	lw r5, [r2 - 1]
	lw r6, [r1 + r2]

	hlt 077

dt1:
.data	0x0001
dt2:
.data	dt3
dt3:
.data	0x0003
dt4:
.data	0x0004

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT hex(r1): 0x0001
; XPCT hex(r2): 0x000d
; XPCT hex(r3): 0x0003
; XPCT hex(r4): 0x000d
; XPCT hex(r5): 0x000d
; XPCT hex(r6): 0x0004

# vim: tabstop=4
