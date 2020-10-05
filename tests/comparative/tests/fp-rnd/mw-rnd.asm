; LIMIT 200
; INPUT Rnd(Flags, 0, 65536)
; INPUT Rnd(Int, -32768, 32768)
; INPUT Rnd(Int, -32769, 32768)
; OUTPUT Flags
; OUTPUT Dint

	lw	r0, [r7]	; load r0
	lw	r2, [r7+1]	; load r2
	mw	r7+2		; execute operation

	rw	r0, r7		; store r0
	rd	r7+1		; store result

	uj	r4
data:
