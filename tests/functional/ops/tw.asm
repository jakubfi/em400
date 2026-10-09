	; map block 1 page 0 to module 1 frame 0, set NB=1
	lwt	r1, 1
	ou	r1, 0b0000000000000011
	.word	err, err, ok, err
ok:
	mb	blk

	; block 1 is not reloaded with the program, set up words used by the test
	lw	r1, 0x5a3c
	pw	r1, 101
	lwt	r1, 0
	pw	r1, 102
	lw	r1, 0x8000
	pw	r1, 103
	lw	r1, 0xa5c3
	pw	r1, 104

	; tw should load from block NB, not touch flags
	lwt	r0, -1
	lwt	r1, 0
	tw	r1, 101
	rpc	r2

	; tw should not set any flags (especially Z on zero, M on negative)
	lwt	r0, 0
	lwt	r3, -1
	tw	r3, 102
	rpc	r4
	lwt	r5, -1
	tw	r5, 103
	rpc	r6

	; tw should load the whole word into r0 in OS mode
	tw	r0, 104

	hlt	077

err:	hlt	040
blk:	.word	1

	.org	101
	.word	0x2222, 0x2222, 0x2222, 0x2222

; XPCT r1 : 0x5a3c
; XPCT r2 : 0xffff
; XPCT r3 : 0
; XPCT r4 : 0
; XPCT r5 : 0x8000
; XPCT r6 : 0
; XPCT r0 : 0xa5c3
