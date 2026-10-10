; PRECMD brk r1==1000

	lwt	r1, 0
loop:	irb	r1, loop
	hlt	040

; XPCT r1 : 1000
; XPCT ic : 1

; POSTCMD brkdel 0
