
	lw	r1, 10
	lw	r2, 20
	lw	r3, 30
	lw	r4, 40
	lw	r5, 50
	lw	r6, 60
	lw	r7, 70

	ra	data

	hlt	077

	.org	20
data:

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int([20]) : 10
; XPCT int([21]) : 20
; XPCT int([22]) : 30
; XPCT int([23]) : 40
; XPCT int([24]) : 50
; XPCT int([25]) : 60
; XPCT int([26]) : 70
