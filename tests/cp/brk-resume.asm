; Resuming after a hit executes the instruction at IC instead of stopping on
; it again, and the breakpoint fires again on the next pass through the loop.

; PRECMD brk ic==1
; RESUME 2

	lwt	r1, 0
loop:	awt	r1, 1
	ujs	loop

; POSTCMD brkdel 0

; XPCT ic : 1
; XPCT r1 : 2
