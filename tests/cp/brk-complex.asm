; PRECMD brk (r1==1000)&&(ic==3)

	lwt	r1, 0
loop:	nop
	nop
	nop
	nop
	irb	r1, loop
	hlt	040

; POSTCMD brkdel 0

; XPCT r1 : 1000
; XPCT ic : 3
; XPCT ir : 0b1110000000000000
