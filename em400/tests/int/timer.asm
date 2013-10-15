.prog "int/timer"

; CONFIG configs/minimal-clock.cfg
; busy loop for a while, waiting for timer to tick

	lwt r1, 0
	lwt r2, -5

loop2:	trb r2, 1
	ujs loop1
	hlt 077
loop1:	trb r1, 1
	ujs loop1
	ujs loop2

.finprog

; XPCT int(sr) : 0
; XPCT int(rz[6]) : 0

; XPCT int(rz[5]) : 1

