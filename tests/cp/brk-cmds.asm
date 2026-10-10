; brken and brkedit change what fires: the disabled breakpoint would stop
; at r1==2, the edited one would never stop as originally set.

; PRECMD brk r1==2
; PRECMD brken 0 off
; PRECMD brk r1==9999
; PRECMD brkedit 1 r1==3

	lwt	r1, 0
loop:	awt	r1, 1
	ujs	loop

; XPCT r1 : 3
; XPCT_CMD brkhit : "OK: 1"
; XPCT_CMD brklist : "OK: 0 1"
; XPCT_CMD brkget 0 : "OK: off r1==2"
; XPCT_CMD brkget 1 : "OK: on r1==3"

; brkeval ignores the enabled state
; XPCT_CMD brkeval 0 : "OK: 0"
; XPCT_CMD brkeval 1 : "OK: 1"

; POSTCMD brkdel 0
; POSTCMD brkdel 1
