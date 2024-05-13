; illegal instruction resets MOD

	lw	r1, 0xffff
	lw	r2, [test]
	rw	r2, 0x1fff
	uj	0x1fff

test:
	lw	r1, [1] ; this will fail when executed @ 0x1fff, since 0x2000 is not configured,
			; but _will_ execute anyway, with [0] as arg, and load "lw r1" as data into r1 (0x4040)

; XPCT rz[6] : 0
; XPCT r1 : 0x4040
