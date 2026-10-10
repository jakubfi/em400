; Register and flag reads. ri leaves different values in AR (the store
; address) and AC (the argument); a breakpoint stops the CPU before hlt
; overwrites them.

; PRECMD reg KB 0xbeef
; PRECMD brk ic==12

	lwt	r1, 1
	lwt	r2, 2
	lwt	r3, 3
	lwt	r4, 4
	lwt	r5, 5
	lwt	r6, 6
	lw	r0, 0b_1100_1101_1000_1000
	lw	r7, 0x200
	ri	r7, 0x5555
	hlt	077

; XPCT ic : 12
; XPCT r1 : 1
; XPCT r2 : 2
; XPCT r3 : 3
; XPCT r4 : 4
; XPCT r5 : 5
; XPCT r6 : 6
; XPCT r7 : 0x201
; XPCT r0 : 0b1100110110001000

; XPCT Z : 1
; XPCT M : 1
; XPCT V : 0
; XPCT C : 0
; XPCT L : 1
; XPCT E : 1
; XPCT G : 0
; XPCT Y : 1
; XPCT X : 1

; XPCT [0x200] : 0x5555
; XPCT ar : 0x200
; XPCT ac : 0x5555
; XPCT kb : 0xbeef
; XPCT ar + ac : 0x5755
; XPCT kb & 0xff : 0xef
; XPCT mc : 0

; POSTCMD brkdel 0
