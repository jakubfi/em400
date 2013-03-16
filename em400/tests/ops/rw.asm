.prog "op/RW"

; PRE r0 = -10
; PRE r1 = -11
; PRE r2 = -12
; PRE r3 = -13
; PRE r4 = -14
; PRE r5 = -15
; PRE r6 = -16
; PRE r7 = -17

	rw r0, 20
	rw r1, 21
	rw r2, 22
	rw r3, 23
	rw r4, 24
	rw r5, 25
	rw r6, 26
	rw r7, 27

	hlt 077

.res	100

.finprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int([20]) : -10
; XPCT int([21]) : -11
; XPCT int([22]) : -12
; XPCT int([23]) : -13
; XPCT int([24]) : -14
; XPCT int([25]) : -15
; XPCT int([26]) : -16
; XPCT int([27]) : -17

