; memory configuration is reset on MCL

	.cpu mera400

	.equ int_nomem 0x40 + 2
	.equ stackp 0x61
	.equ segment 1
	.equ page 0
	.equ module 1
	.equ frame 3

	uj	start

ba:	.word	0b0100000000000000 + segment\15
stack:	.res	16

nomem_proc:
	hlt	045

	.org	0x70

start:	lwt	r1, stack
	rw	r1, stackp
	lwt	r1, nomem_proc
	rw	r1, int_nomem

	lw	r1, page\3 + segment\15
	lw	r2, frame\10 + module\14 + 1
	ou	r1, r2
	.word	err, err, ok, err
err:	hlt	044

ok:	mb	ba
	im	ba
	lwt	r6, 0
	lwt	r7, 9

	; this should succeed, segment is configured
	tw	r1, 1
	lwt	r6, 10

	mcl
	mb	ba
	im	ba

	; this should fail and stop CPU with an alarm...
	tw	r1, 1
	; ...right here
	lwt	r7, 10

	hlt	046

; XPCT r6 : 10
; XPCT r7 : 9
