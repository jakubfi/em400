; Memory expressions in the evaluator: register-indexed, computed-address and
; indirect reads - the forms users actually write for watches/breakpoints
; (e.g. [r1+12], [[ptr]]).

; PRECMD memw 0 200 201 4660 9029
; PRECMD reg r1 200
; PRECMD reg r2 1

	hlt	077

; register-indexed reads
; XPCT [r1] : 201
; XPCT [r1+1] : 4660
; XPCT [r1+2] : 9029

; computed address
; XPCT [r1+r2] : 4660
; XPCT [r1+2-1] : 4660

; indirect
; XPCT [[200]] : 4660
; XPCT [[r1]] : 4660

; explicit segment with a register address
; XPCT [0:r1] : 201
; XPCT [0:r1+2] : 9029

; base handling on the result (201 == 0xc9)
; XPCT [r1] : 0xc9
