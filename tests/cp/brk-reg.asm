; PRECMD brk r1==1000

	lwt	r1, 0
loop:	irb	r1, loop
	hlt	040

; POSTCMD brkdel 0

; XPCT r1 : 1000
; XPCT ic : 1
