; pre-modification fails on 4th time

	lw	r2, 1
	md	1
	md	1
	md	1
	md	r0	; this should cause 'illegal instruction'...
	lwt	r2, 7	; ...but this should be executed anyway,
			; despite being really an argument to the instruction above
			; due to the fact that illegal instructions are ended before
			; the 16-bit argument is fetched

	hlt	077

; XPCT sr : 0

; XPCT rz[6] : 1
; XPCT r2 : 7
