; Evaluator error paths: each expression must be rejected with the given message.
; The (at b-e) column range must span exactly the failing subexpression.

	hlt	077

; XPCT_ERR 1/0 : "Division by zero (at 0-2)"
; XPCT_ERR 1/(2-2) : "Division by zero (at 0-6)"
; XPCT_ERR [16:0] : "Wrong memory segment: 16 (at 1-2)"
; XPCT_ERR [ 16 : 0] : "Wrong memory segment: 16 (at 2-3)"
; XPCT_ERR [0xffff:0] : "Wrong memory segment: 65535 (at 1-6)"
; XPCT_ERR [16:[0]+1] : "Wrong memory segment: 16 (at 1-2)"
; XPCT_ERR [15:0] : "Memory at 15:0 is not configured (at 0-5)"
; XPCT_ERR [1:0] : "Memory at 1:0 is not configured (at 0-4)"
; XPCT_ERR rz[32] : "Wrong interrupt: 32 (at 0-5)"
; XPCT_ERR 1+ : "incomplete expression"
; XPCT_ERR (1 : "incomplete expression"
; XPCT_ERR foo : "Invalid input: 'foo' (at 0-2)"
; XPCT_ERR 1 $ : "Invalid input: '$' (at 2-2)"
; XPCT_ERR 1 2 : "unexpected value (at 2-2)"

; number literals must fit in 16 bits
; XPCT_ERR 0x10000 : "Value out of range (at 0-6)"
; XPCT_ERR 65536 : "Value out of range (at 0-4)"
; XPCT_ERR 0200000 : "Value out of range (at 0-6)"
; XPCT_ERR 0b10000000000000000 : "Value out of range (at 0-18)"
; XPCT_ERR 99999999999999999999999 : "Value out of range (at 0-22)"
; XPCT_ERR 1 + 0x10000 : "Value out of range (at 4-10)"
; XPCT_ERR [65536:100] : "Value out of range (at 1-5)"
; XPCT_ERR [0x100000000:0] : "Value out of range (at 1-11)"
; XPCT_ERR [0:0x10000] : "Value out of range (at 3-9)"
; XPCT_ERR rz[0x100000020] : "Value out of range (at 3-13)"
; XPCT_ERR @0:0x10000 : "Value out of range (at 3-9)"

; errors propagate up through operators, the range stays on the failing part
; XPCT_ERR 1 + 1/0 : "Division by zero (at 4-6)"
; XPCT_ERR 1 + 1/(2-2) : "Division by zero (at 4-10)"
; XPCT_ERR -(1/0) : "Division by zero (at 2-4)"
; XPCT_ERR 1 + [1:5]*2 : "Memory at 1:5 is not configured (at 4-8)"

; errors in a memory address are not masked by the outer read
; XPCT_ERR [[16:0]] : "Wrong memory segment: 16 (at 2-3)"
; XPCT_ERR [1/0] : "Division by zero (at 1-3)"
; XPCT_ERR [0:1/0] : "Division by zero (at 3-5)"
; XPCT_ERR [[1:0]+1] : "Memory at 1:0 is not configured (at 1-5)"

; message is optional
; XPCT_ERR 5/0
