.prog "op/MCL"

; PRE sr = 0b0000001000000000
; PRE r0 = 0xfafa
; PRE r1 = 12
; PRE r2 = 41
; PRE r3 = 523
; PRE r4 = 5347
; PRE r5 = 1
; PRE r6 = 236
; PRE r7 = -24886

	.data 1
	mcl
	hlt 077

.finprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT int(r0) : 0
; XPCT int(r1) : 12
; XPCT int(r2) : 41
; XPCT int(r3) : 523
; XPCT int(r4) : 5347
; XPCT int(r5) : 1
; XPCT int(r6) : 236
; XPCT int(r7) : -24886

