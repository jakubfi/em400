; OPTS -c configs/minimal.cfg

; PRECMD reg r0 1
; PRECMD reg r1 0xffff
; PRECMD reg r2 0xbaba
; PRECMD reg r3 12345
; PRECMD reg r4 -2
; PRECMD reg r5 -10
; PRECMD reg r6 -32000
; PRECMD reg r7 0
; PRECMD reg KB 9
; PRECMD reg IC 200
; PRECMD reg SR 11

	hlt	040

	.org	200
	hlt	077

; XPCT r0 : 1
; XPCT r1 : -1
; XPCT r2 : 0xbaba
; XPCT r3 : 12345
; XPCT r4 : 0xfffe
; XPCT r5 : -10
; XPCT r6 : -32000
; XPCT r7 : 0
; XPCT KB : 9
; XPCT ir&0x3f : 0o77
; XPCT IC : 201
; XPCT SR : 11
