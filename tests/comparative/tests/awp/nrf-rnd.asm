; LIMIT 500
; INPUT Rnd(0, 65536, 1)
; INPUT Rnd(0, 65536, 1)
; INPUT Rnd(0, 65536, 1)
; INPUT Rnd(0, 65536, 1)
; OUTPUT 5

	ric	r7
	awt	r7, data-.

	rz	r7+100		; clear interrupts
	fi	r7+100

	lw	r0, [r7]	; load r0

	lf	r7+1		; load floating point argument

	nrf			; normalize

	rw	r0, r7		; store r0
	rf	r7+1		; store result
	ki	r7+4		; store interrupts

	uj	r4
data:
