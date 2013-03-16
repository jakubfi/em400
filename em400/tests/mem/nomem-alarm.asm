.prog "mem/nomem-alarm"

; CONFIG configs/no_user_mem.cfg

; PRE sr = 0b0000000000000001

	pw r1, 10

	hlt 077

.finprog

; XPCT int(rz(2)) : 1
; XPCT int(rz(6)) : 0
; XPCT int(alarm) : 1
