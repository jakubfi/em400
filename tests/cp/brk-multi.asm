; Several breakpoints installed at once: CPU must stop on whichever
; condition is satisfied first, the others (including one that never fires)
; must coexist harmlessly.

; PRECMD brk r1==30
; PRECMD brk r2==50
; PRECMD brk r1==9999

	lwt	r1, 0
	lwt	r2, 0
loop:	awt	r1, 1
	awt	r2, 2
	ujs	loop
	hlt	040

; POSTCMD brkdel 0
; POSTCMD brkdel 1
; POSTCMD brkdel 2

; XPCT r1 : 25
; XPCT r2 : 50
