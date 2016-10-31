; PRECMD brk (r1==1000)&&(ic==3)

	LWT	r1, 0
loop:	NOP
	NOP
	NOP
	NOP
	IRB	r1, loop
	HLT	040

; POSTCMD brkdel 0

; XPCT r1 : 1000
; XPCT ic : 3
; XPCT ir : 0b1110000000000000
