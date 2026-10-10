	; sets X
	lw	r0, 0x8000
	sxu	r0
	rpc	r1

	; clears X
	lw	r0, ?X
	sxu	r0
	rpc	r2

	hlt	077

; XPCT r1 : 0x8080
; XPCT r2 : 0
