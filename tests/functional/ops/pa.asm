	; pa should store r1-r7 at consecutive addresses, not touch flags or neighbouring words
	lwt	r0, -1
	lw	r1, 0x5a3c
	lw	r2, 0xa5c3
	lw	r3, 0x0f0f
	lw	r4, 0xf0f0
	lw	r5, 0x00ff
	lw	r6, 0xff00
	lw	r7, 0x1234
	pa	101
	rw	r0, 400

	; pa should not set Z on zero, M on negative
	lwt	r0, 0
	lwt	r1, 0
	lw	r2, 0x8000
	pa	201
	rw	r0, 401

	; pa should store the register used as the argument, not touch registers
	lw	r1, 301
	pa	r1

	hlt	077

	.org	100
	.word	-1, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, -1
	.org	201
	.word	-1, -1, -1, -1, -1, -1, -1
	.org	301
	.word	-1, -1, -1, -1, -1, -1, -1
	.org	400
	.word	0x1111, 0x1111

; XPCT [100] : 0xffff
; XPCT [101] : 0x5a3c
; XPCT [102] : 0xa5c3
; XPCT [103] : 0x0f0f
; XPCT [104] : 0xf0f0
; XPCT [105] : 0x00ff
; XPCT [106] : 0xff00
; XPCT [107] : 0x1234
; XPCT [108] : 0xffff
; XPCT [400] : 0xffff
; XPCT [201] : 0
; XPCT [202] : 0x8000
; XPCT [207] : 0x1234
; XPCT [401] : 0
; XPCT [301] : 301
; XPCT [302] : 0x8000
; XPCT [307] : 0x1234
; XPCT r1 : 301
; XPCT r2 : 0x8000
; XPCT r3 : 0x0f0f
; XPCT r4 : 0xf0f0
; XPCT r5 : 0x00ff
; XPCT r6 : 0xff00
; XPCT r7 : 0x1234
