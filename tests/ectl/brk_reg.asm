; PRECMD brk r1==1000

	LWT	r1, 0
loop:	IRB	r1, loop
	HLT	040

; POSTCMD brkdel 0

; XPCT r1 : 1000
; XPCT ic : 1
