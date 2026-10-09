	; ls should take argument bits where r7 is set, keep register bits where r7 is clear
	; ls should not touch flags
	lwt	r0, -1
	lw	r1, 0b0101010101010101
	lw	r7, 0b0000111111110000
	ls	r1, 0b0011001100110011
	rpc	r2

	; ls should not set any flags (Z on zero result)
	lwt	r0, 0
	lw	r3, 0b0000111111110000
	ls	r3, 0b1111000000001111
	rpc	r4

	; ls into r7 should use r7 before the write as the mask
	ls	r7, 0b0011001100110011

	hlt	077

; XPCT r1 : 0b0101001100110101
; XPCT r2 : 0xffff
; XPCT r3 : 0
; XPCT r4 : 0
; XPCT r7 : 0b0000001100110000
