
	MB	data
	LW	r1, 1
	LW	r2, 2
	LW	r3, 3
	LW	r4, 4
	LW	r5, 5
	LW	r6, 6
	LW	r7, 7
	LW	r0, 0b1100110110001000
	HLT	077

data:	.word 0b0000000000000111

	.org 100
	.word 0xdead, 0xbeef

; XPCT -3 : 0xfffd
; XPCT 0xffff : -1
; XPCT 0b1001 : 9
; XPCT 010 : 8

; XPCT r1 : 1
; XPCT r2 : 2
; XPCT r3 : 3
; XPCT r4 : 4
; XPCT r5 : 5
; XPCT r6 : 6
; XPCT r7 : 7
; XPCT r0 : 0b1100110110001000
; XPCT sr : 7

; XPCT Z : 1
; XPCT M : 1
; XPCT V : 0
; XPCT C : 0
; XPCT L : 1
; XPCT E : 1
; XPCT G : 0
; XPCT Y : 1
; XPCT X : 1

; XPCT [100] : 0xdead
; XPCT [0:101] : 0xbeef

; XPCT -1 : 0xffff
; XPCT 1+2+3+4 : 10
; XPCT 9-4-2-1 : 2
; XPCT 1*2*3*4 : 24
; XPCT 100/2/2/5 : 5

; XPCT 0b1100 ^ 0b0110 : 0b1010
; XPCT 0b1100 | 0b0110 : 0b1110
; XPCT 0b1100 & 0b0110 : 0b0100

; XPCT 2<<2 : 8
; XPCT 16>>2 : 4

; XPCT 1||0||0||0||0 : 1
; XPCT 0||0||0||0||1 : 1
; XPCT 0||0||1||0||0 : 1
; XPCT 0||0||0||0||0 : 0
; XPCT 1&&1&&1&&1&&1 : 1
; XPCT 0&&1&&1&&1&&1 : 0
; XPCT 1&&1&&1&&1&&0 : 0
; XPCT 1&&1&&0&&1&&1 : 0

; XPCT 1000==1000 : 1
; XPCT 1000<=1000 : 1
; XPCT 1000>=1000 : 1
; XPCT 1000!=1001 : 1
; XPCT 1000!=1000 : 0
; XPCT 1>2 : 0
; XPCT 1<2 : 1

; XPCT ~1 : 0xfffe
; XPCT ~0b1100101101101001 : 0b0011010010010110

; XPCT !1 : 0
; XPCT !0 : 1
; XPCT !10 : 0

; XPCT 2+2*2 : 6
; XPCT (2+2)*2 : 8
; XPCT (2+2)*2*(4-2) : 16

