.program "op/LS"

; PRE r1 = 0b1111000011110000
; PRE r7 = 0b1111111100000000

	ls r1, 0b0000111100001111

	hlt 077

.endprog

; XPCT int(rz(6)) : 0
; XPCT int(sr) : 0

; XPCT bin(r1): 0b0000111111110000

