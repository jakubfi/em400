; Evaluator error paths: each expression must be rejected with the given message.

	hlt	077

; XPCT_ERR 1/0 : "Division by zero"
; XPCT_ERR 1/(2-2) : "Division by zero"
; XPCT_ERR [16:0] : "Wrong memory segment: 16"
; XPCT_ERR [1:0] : "Memory at 1:0 is not configured"
; XPCT_ERR rz[32] : "Wrong interrupt: 32"
; XPCT_ERR 1+ : "incomplete expression"
; XPCT_ERR (1 : "incomplete expression"
; XPCT_ERR foo : "Invalid input: 'foo' (at 0-2)"
; XPCT_ERR 1 $ : "Invalid input: '$' (at 2-2)"
; XPCT_ERR 1 2 : "unexpected value"

; errors propagate up through operators
; XPCT_ERR 1 + 1/0 : "Division by zero"
; XPCT_ERR -(1/0) : "Division by zero"

; message is optional
; XPCT_ERR 5/0
