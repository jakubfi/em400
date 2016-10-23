; OPTS -c configs/minimal-clock.cfg
; busy loop for a while, waiting for the timer to tick

	lwt r1, 0
	lwt r2, -20

loop2:	trb r2, 1
	ujs loop1
	hlt 077
loop1:	trb r1, 1
	ujs loop1
	ujs loop2

; XPCT sr : 0
; XPCT rz[6] : 0

; XPCT rz[5] : 1
