; 5/2
; PRE [0xa0] = 0b0000000000000000
; PRE [0xa1] =                   0b0000000000000101
; PRE [0xa2] = 0b0000000000000010
; 4/2
; PRE [0xa3] = 0b0000000000000000
; PRE [0xa4] =                   0b0000000000000100
; PRE [0xa5] = 0b0000000000000010
; 39746371/1213
; PRE [0xa6] = 0b0000001001011110
; PRE [0xa7] =                   0b0111101101000011
; PRE [0xa8] = 0b0000010010111101
; 19/-4
; PRE [0xa9] = 0b0000000000000000
; PRE [0xaa] =                   0b0000000000010011
; PRE [0xab] = 0b1111111111111100
; -33/5
; PRE [0xac] = 0b1111111111111111
; PRE [0xad] =                   0b1111111111011111
; PRE [0xae] = 0b0000000000000101
; 0/22
; PRE [0xaf] = 0b0000000000000000
; PRE [0xb0] =                   0b0000000000000000
; PRE [0xb1] = 0b0000000000010110
; -39746371/1213
; PRE [0xb2] = 0b1111110110100001
; PRE [0xb3] =                   0b1000000000000000
; PRE [0xb4] = 0b0000010010111101

	ld 0xa0
	dw 0xa2
	rd 0xe0
	rw r0, 0xe2

	ld 0xa3
	dw 0xa5
	rd 0xe3
	rw r0, 0xe5

	ld 0xa6
	dw 0xa8
	rd 0xe6
	rw r0, 0xe8

	ld 0xa9
	dw 0xab
	rd 0xe9
	rw r0, 0xeb

	ld 0xac
	dw 0xae
	rd 0xec
	rw r0, 0xee

	ld 0xaf
	dw 0xb1
	rd 0xef
	rw r0, 0xf1

	ld 0xb2
	dw 0xb4
	rd 0xf2
	rw r0, 0xf4

	hlt 077

; XPCT int(rz[6]) : 0
; XPCT int(rz[7]) : 0
; XPCT int(rz[8]) : 0
; XPCT int(rz[9]) : 0
; XPCT int(rz[10]) : 0
; XPCT int(sr) : 0

; XPCT bin([0xe0]) : 0b0000000000000001
; XPCT bin([0xe1]) :                   0b0000000000000010
; XPCT bin([0xe2]) : 0b0000000000000000

; XPCT bin([0xe3]) : 0b0000000000000000
; XPCT bin([0xe4]) :                   0b0000000000000010
; XPCT bin([0xe5]) : 0b0000000000000000

; XPCT bin([0xe6]) : 0b0000000000000000
; XPCT bin([0xe7]) :                   0b0111111111111111
; XPCT bin([0xe8]) : 0b0000000000000000

; XPCT bin([0xe9]) : 0b0000000000000011
; XPCT bin([0xea]) :                   0b1111111111111100
; XPCT bin([0xeb]) : 0b0100000000000000

; XPCT bin([0xec]) : 0b1111111111111101
; XPCT bin([0xed]) :                   0b1111111111111010
; XPCT bin([0xee]) : 0b0100000000000000

; XPCT bin([0xef]) : 0b0000000000000000
; XPCT bin([0xf0]) :                   0b0000000000000000
; XPCT bin([0xf1]) : 0b1000000000000000

; XPCT bin([0xf2]) : 0b0000000000000000
; XPCT bin([0xf3]) :                   0b1000000000000000
; XPCT bin([0xf4]) : 0b0100000000000000

