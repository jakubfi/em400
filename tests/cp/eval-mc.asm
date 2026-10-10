; mc counts consecutive md pre-modifications and is cleared by the next
; non-md instruction, so the breakpoint stops right after the last md.

; PRECMD brk ic==6

	md	0x1000
	md	0x0200
	md	0x0034
	hlt	077

; POSTCMD brkdel 0

; XPCT ic : 6
; XPCT mc : 3
; XPCT ar : 0x1234
; XPCT ac : 0x1234
