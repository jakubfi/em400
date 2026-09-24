
	lw	r1, 0xffff
	lw	r2, [test]
	rw	r2, 0x1fff
	uj	0x1fff

test:
	; this will fail when executed @ 0x1fff, since memory @ 0x2000 is not configured,
	; but _will_ execute anyway, with [0] as the arg,
	; and load "lw r1" opcode as data into r1 (0x4040)
	lw	r1, [1]

; XPCT rz[6] : 0
; XPCT r1 : 0x4040
