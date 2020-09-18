	; sets ZC
	lw	r0, 0
	nga	r0
	rpc	r1

	; clears ZC
	lw	r0, 1
	nga	r0
	rpc	r2

	; sets M
	lw	r0, 0b0111111111111111
	nga	r0
	rpc	r3

	; clears M
	lw	r0, 0b1000000000000001
	nga	r0
	rpc	r4

	; sets V
	lw	r0, 0b1000000000000000
	nga	r0
	rpc	r5

	; V is never cleared by nga

	hlt	077

; XPCT r1 : 0
; XPCT r2 : -1
; XPCT r3 : 0b1000000000000001
; XPCT r4 : 0b0111111111111111
; XPCT r5 : 0b1000000000000000
