; Error paths of brken, brkedit, brkeval, brkget and brklist.

; PRECMD brk r1==1
; PRECMD brk [1:0]

	hlt	077

; XPCT_CMD brken : "ERR: Missing argument (breakpoint number)"
; XPCT_CMD brken 0 : "ERR: Missing argument (state)"
; XPCT_CMD brken 0 maybe : "ERR: Wrong state: maybe"
; XPCT_CMD brken 9 on : "ERR: No such breakpoint"

; XPCT_CMD brkedit : "ERR: Missing argument (breakpoint number)"
; XPCT_CMD brkedit 0 : "ERR: Missing argument (expression)"
; XPCT_CMD brkedit 9 r1 : "ERR: No such breakpoint"
; XPCT_CMD brkedit 0 1+ : "ERR: incomplete expression (at 1-1)"
; XPCT_CMD brkget 0 : "OK: on r1==1"

; XPCT_CMD brkeval : "ERR: Missing argument (breakpoint number)"
; XPCT_CMD brkeval 9 : "ERR: No such breakpoint"
; XPCT_CMD brkeval 1 : "ERR: Memory at 1:0 is not configured (at 0-4)"

; XPCT_CMD brkget : "ERR: Missing argument (breakpoint number)"
; XPCT_CMD brkget 9 : "ERR: No such breakpoint"

; deleted breakpoints are gone for every command
; XPCT_CMD brkdel 0 : "OK: Removed breakpoint: 0"
; XPCT_CMD brklist : "OK: 1"
; XPCT_CMD brkget 0 : "ERR: No such breakpoint"
; XPCT_CMD brken 0 on : "ERR: No such breakpoint"
; XPCT_CMD brkedit 0 r1 : "ERR: No such breakpoint"
; XPCT_CMD brkeval 0 : "ERR: No such breakpoint"
; XPCT_CMD brkdel 1 : "OK: Removed breakpoint: 1"
; XPCT_CMD brklist : "OK:"
