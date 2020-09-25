; LIMIT 500
; INPUT Rnd(0, 65536, 1)
; INPUT Rnd(0, 65536, 1)
; INPUT Rnd(0, 65536, 1)
; INPUT Rnd(0, 65536, 1)
; INPUT Rnd(0, 65536, 1)
; INPUT Rnd(0, 65536, 1)
; INPUT Rnd(0, 65536, 1)
; OUTPUT 5

	ric	r7
	awt	r7, data-.

	lw	r0, [r7]	; load r0

	lf	r7+4		; normalize mem arg
	nrf
	rf	r7+4

	lf	r7+1		; load and normalize r arg
	nrf

	rz	r7+100		; clear interrupts
	fi	r7+100

	af	r7+4		; execute operation

	rw	r0, r7		; store r0
	rf	r7+1		; store result
	ki	r7+4		; store interrupts

	uj	r4
data:
