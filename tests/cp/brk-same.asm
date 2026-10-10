; Breakpoints are checked newest first, so when several are true on the same
; instruction the most recently added one is reported.

; PRECMD brk r1==2
; PRECMD brk r1>=2
; PRECMD brk r1==9999

	lwt	r1, 0
loop:	awt	r1, 1
	ujs	loop

; XPCT r1 : 2
; XPCT_CMD brkhit : "OK: 1"

; POSTCMD brkdel 0
; POSTCMD brkdel 1
; POSTCMD brkdel 2
