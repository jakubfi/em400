.prog "cycle/pre-mod-reset"

; legal instruction resets MOD

	md 1
	md 1
	md 1

	hlt 074

.finprog

; XPCT int(sr) : 0
; XPCT int(rz[6]) : 0

; XPCT int(mod) : 0

