	; map block 1 page 0 to module 1 frame 0, set NB=1
	lwt	r1, 1
	ou	r1, 0b0000000000000011
	.word	err, err, ok, err
ok:
	mb	blk

	; block 1 is not reloaded with the program, clear words checked by the test
	lwt	r1, -1
	lw	r2, -300
clr:	pw	r1, r2+400
	irb	r2, clr

	; pa should store r1-r7 at consecutive addresses in block NB, not touch flags, neighbouring words or block 0
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

err:	hlt	040
blk:	.word	1

	.org	101
	.word	0x2222, 0x2222, 0x2222, 0x2222, 0x2222, 0x2222, 0x2222
	.org	400
	.word	0x1111, 0x1111

; XPCT [1:100] : 0xffff
; XPCT [1:101] : 0x5a3c
; XPCT [1:102] : 0xa5c3
; XPCT [1:103] : 0x0f0f
; XPCT [1:104] : 0xf0f0
; XPCT [1:105] : 0x00ff
; XPCT [1:106] : 0xff00
; XPCT [1:107] : 0x1234
; XPCT [1:108] : 0xffff
; XPCT [101] : 0x2222
; XPCT [102] : 0x2222
; XPCT [103] : 0x2222
; XPCT [104] : 0x2222
; XPCT [105] : 0x2222
; XPCT [106] : 0x2222
; XPCT [107] : 0x2222
; XPCT [400] : 0xffff
; XPCT [1:201] : 0
; XPCT [1:202] : 0x8000
; XPCT [1:203] : 0x0f0f
; XPCT [1:204] : 0xf0f0
; XPCT [1:205] : 0x00ff
; XPCT [1:206] : 0xff00
; XPCT [1:207] : 0x1234
; XPCT [401] : 0
; XPCT [1:301] : 301
; XPCT [1:302] : 0x8000
; XPCT [1:303] : 0x0f0f
; XPCT [1:304] : 0xf0f0
; XPCT [1:305] : 0x00ff
; XPCT [1:306] : 0xff00
; XPCT [1:307] : 0x1234
; XPCT r1 : 301
; XPCT r2 : 0x8000
; XPCT r3 : 0x0f0f
; XPCT r4 : 0xf0f0
; XPCT r5 : 0x00ff
; XPCT r6 : 0xff00
; XPCT r7 : 0x1234
