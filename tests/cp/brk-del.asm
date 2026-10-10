; brkdel error paths and id allocation: ids are not reused while any
; breakpoint exists, and restart from 0 once the list is empty.

; PRECMD brk r1==9999
; PRECMD brk r1==9999

	hlt	077

; XPCT_CMD brkdel 5 : "ERR: No such breakpoint"
; XPCT_CMD brkdel -1 : "ERR: No such breakpoint"
; XPCT_CMD brkdel foo : "ERR: No such breakpoint"
; XPCT_CMD brkdel : "ERR: Missing argument (breakpoint number)"

; XPCT_CMD brkdel 0 : "OK: Removed breakpoint: 0"
; XPCT_CMD brkdel 0 : "ERR: No such breakpoint"
; XPCT_CMD brk r1==9999 : "OK: Added breakpoint: 2"

; XPCT_CMD brkdel 1 : "OK: Removed breakpoint: 1"
; XPCT_CMD brkdel 2 : "OK: Removed breakpoint: 2"
; XPCT_CMD brk r1==9999 : "OK: Added breakpoint: 0"
; XPCT_CMD brkdel 0 : "OK: Removed breakpoint: 0"
