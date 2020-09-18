	; sets Z
	lw	r0, 0xffff
	ngc	r0
	rpc	r1

	; clears ZC
	lw	r0, 1
	ngc	r0
	rpc	r2

	; NGC 0 sets C, but result also sets C, nothing to test

	; no way to test V

	hlt	077

; XPCT r1 : 1
; XPCT r2 : 0xfffe
