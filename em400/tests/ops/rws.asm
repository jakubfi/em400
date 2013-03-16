.prog "op/RWS"

; PRE r0 = -30
; PRE r1 = -31
; PRE r2 = -32
; PRE r3 = -33
; PRE r4 = -34
; PRE r5 = -35
; PRE r6 = -36
; PRE r7 = -37

	ujs main

data1:	.res	9

main:	rws r0, data1
	rws r1, data1+1
	rws r2, data1+2
	rws r3, data1+3
	rws r4, data2
	rws r5, data2+1
	rws r6, data2+2
	rws r7, data2+3

	hlt 077
	nop

data2:	.res	100

.finprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int([1]) : -30
; XPCT int([2]) : -31
; XPCT int([3]) : -32
; XPCT int([4]) : -33
; XPCT int([20]) : -34
; XPCT int([21]) : -35
; XPCT int([22]) : -36
; XPCT int([23]) : -37

