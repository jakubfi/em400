	; without carry ngc is a bitwise negation
	lwt	r0, 0
	lw	r1, 0x5a3c
	ngc	r1

	; with carry ngc is an arithmetic negation
	lw	r0, ?C
	lw	r2, 0x5a3c
	ngc	r2

	hlt	077

; XPCT r1 : 0xa5c3
; XPCT r2 : 0xa5c4
