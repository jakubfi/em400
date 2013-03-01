.program "cpu/pre-mod"

	md 100
	lw r1, 1
	md -1
	lw r2, 1
	md -16
	lw r3, 0
	md -1024
	lw r4, -1024
	md 0xffff
	lw r5, 1

	hlt 077

.endprog

; XPCT int(r1): 101
; XPCT int(r2): 0
; XPCT int(r3): -16
; XPCT int(r4): -2048
; XPCT int(r5): 0

# vim: tabstop=4
