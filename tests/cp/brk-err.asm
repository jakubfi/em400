; A condition that fails to evaluate counts as false: that breakpoint never
; fires, and breakpoints checked before or after it keep working.

; PRECMD brk [1:0]==0
; PRECMD brk r1==3
; PRECMD brk 1/(r1-r1)

	lwt	r1, 0
loop:	awt	r1, 1
	ujs	loop

; POSTCMD brkdel 0
; POSTCMD brkdel 1
; POSTCMD brkdel 2

; XPCT r1 : 3
