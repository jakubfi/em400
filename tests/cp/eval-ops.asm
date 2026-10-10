; Number literals and operators, independent of CPU state.

	hlt	077

; XPCT -3 : 0xfffd
; XPCT 0xffff : -1
; XPCT 0b1001 : 9
; XPCT 010 : 8
; XPCT 65535 : 0xffff
; XPCT 0177777 : 0xffff
; XPCT 0b1111111111111111 : 0xffff
; XPCT 0x0000ffff : 0xffff

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
; XPCT 8>>1<<2 : 16
; XPCT 16>>2<<1 : 8
; XPCT 1<<15 : 0x8000
; XPCT 0xffff<<15 : 0x8000
; XPCT 0x8000>>15 : 1
; XPCT 1<<16 : 0
; XPCT 0xffff>>16 : 0
; XPCT 1<<40 : 0
; XPCT 0xffff>>40 : 0
; XPCT 0xffff<<0xffff : 0

; XPCT 1||0||0||0||0 : 1
; XPCT 0||0||0||0||1 : 1
; XPCT 0||0||1||0||0 : 1
; XPCT 0||0||0||0||0 : 0
; XPCT 1&&1&&1&&1&&1 : 1
; XPCT 0&&1&&1&&1&&1 : 0
; XPCT 1&&1&&1&&1&&0 : 0
; XPCT 1&&1&&0&&1&&1 : 0

; short-circuit: the right operand is not evaluated
; XPCT 0 && 1/0 : 0
; XPCT 1 || 1/0 : 1
; XPCT 0 && [1:0] : 0
; XPCT 1 || [1:0] : 1
; XPCT 0 && 1/0 || 1 : 1
; XPCT 1 || 0 && 1/0 : 1
; XPCT 1 || 1/0 || [1:0] : 1
; XPCT 0 && (1/0 || 1) : 0

; XPCT 1000==1000 : 1
; XPCT 1000<=1000 : 1
; XPCT 1000>=1000 : 1
; XPCT 1000!=1001 : 1
; XPCT 1000!=1000 : 0
; XPCT 1000<>1001 : 1
; XPCT 1000<>1000 : 0
; XPCT 1>2 : 0
; XPCT 1<2 : 1
; XPCT 2==2!=2 : 1
; XPCT 1==2!=0 : 0

; XPCT ~1 : 0xfffe
; XPCT ~0b1100101101101001 : 0b0011010010010110

; XPCT !1 : 0
; XPCT !0 : 1
; XPCT !10 : 0

; XPCT 2+2*2 : 6
; XPCT (2+2)*2 : 8
; XPCT (2+2)*2*(4-2) : 16
