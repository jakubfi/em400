.prog "op/RZ"

; PRE [6] = -1
; PRE [7] = -1
; PRE [8] = -1
; PRE [9] = -1
; PRE [10] = -1
; PRE [11] = -1
; PRE [12] = -1
; PRE [13] = -1
; PRE [14] = -1
; PRE [15] = -1
; PRE [16] = -1

	rz 10

	hlt 077

.finprog

; XPCT int(rz[6]) : 0
; XPCT int(sr) : 0

; XPCT int([6]) : -1
; XPCT int([7]) : -1
; XPCT int([8]) : -1
; XPCT int([9]) : -1
; XPCT int([10]) : 0
; XPCT int([12]) : -1
; XPCT int([13]) : -1
; XPCT int([14]) : -1
; XPCT int([15]) : -1
; XPCT int([16]) : -1

