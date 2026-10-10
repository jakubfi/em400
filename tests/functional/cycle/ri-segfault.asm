	; ri should increment the register even if the memory write failed
	; CPU should stop only after ri completes
	lw	r1, 0x2000
	ri	r1, 0x1234
	hlt	040

; XPCT alarm : 1
; XPCT rz[2] : 1
; XPCT r1 : 0x2001
; XPCT ic : 4
