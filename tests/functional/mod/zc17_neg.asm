; OPTS -c configs/mod.cfg

; 17th bit for byte addressing is not set when
; negative pre-mod causes carry=1 when added to rC+rB

	.cpu	mx16

	.const	int_nomem 0x40 + 2
	.const	stackp 0x61

	.const	magic 0xfeba
	.const	addr 0x200
	.const	seg 4

	uj	start

mask:	.word	0b0100000000000000
stack:	.res	16
zeroreg:.word   0, 0, 0, 0, 0, 0, 0

nomem_proc:
	hlt	040

	.org	0x70

start:	la	zeroreg
	lw	r1, stack
	rw	r1, stackp
	lw	r1, nomem_proc
	rw	r1, int_nomem

	lw	r1, seg\3 + 0\15
	ou	r1, 3\10 + 0\14 + 1
	.word	err, err, ok, err

err:	hlt	040

ok:	im	mask

	lw	r1, magic
	rw	r1, seg\3+addr-1

	cron

	lw	r7, seg\3+addr
	md	-1
	lb	r2, r7+r7	; this should work just fine

	hlt	077

; XPCT r2 : 0x00ba
; XPCT ir : 0xec3f
