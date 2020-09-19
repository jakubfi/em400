	; sets VY
	lw	r0, 0b1000_000_00_0000000
	svy	r0
	rpc	r1

	; sets V
	lw	r0, 0b0100_000_00_0000000
	svy	r0
	rpc	r2

	; sets Y
	lw	r0, 0b1100_000_00_0000000
	svy	r0
	rpc	r3

	; clears Y
	lw	r0, 0b0000_000_01_0000000
	svy	r0
	rpc	r4

	hlt	077

; XPCT r1 : 0
; XPCT r2 : 0b1000_000_00_0000000
; XPCT r3 : 0b1000_000_00_0000000
; XPCT r4 : 0b0000_000_10_0000000
