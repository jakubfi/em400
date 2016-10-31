; OPTS -c configs/minimal.cfg

; PRECMD memw 0 100 1 2 3 4 5 6

	HLT 077

; XPCT [100] : 1
; XPCT [101] : 2
; XPCT [102] : 3
; XPCT [103] : 4
; XPCT [104] : 5
; XPCT [105] : 6
