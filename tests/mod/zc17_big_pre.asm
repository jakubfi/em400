; OPTS -c configs/mod.cfg

; 17-bit byte addressing works only when:
;  * cpu_mod is on (MX-16 CPU)
;  * modifications are enabled with CRON
;  * one of the following happened:
;    * carry on pre-mod, no b-mod
;    * carry on b-mod

	.cpu	mx16

	.const	int_nomem 0x40 + 2
	.const	stackp 0x61

	.const	magic1 0x4455
	.const	magic2 0xfeba
	.const	addr 0x200
	.const	seg 9

	uj	start

mask:	.word	0b0100000000000000
stack:	.res	16

nomem_proc:
	hlt	040

	.org	0x70

start:	lw	r1, stack
	rw	r1, stackp
	lw	r1, nomem_proc
	rw	r1, int_nomem

	lw	r1, seg\3 + 0\15
	ou	r1, 3\10 + 0\14 + 1
	.word	err, err, ok, err

err:	hlt	040

ok:	im	mask
	lw	r1, magic1
	rw	r1, (seg\3+addr) & 0b0111111111111111
	lw	r1, magic2
	rw	r1, seg\3+addr
	lw	r7, seg\3+addr

	cron			; enable cpu modification

	lw	r1, 1 + ((seg\3+addr) >> 1)
	md	(seg\3+addr) >> 1
	lb	r3, r7+r1	; this should work (17th bit overflown at B-mod)

	hlt	077

; XPCT hex(r3) : 0x00ba
; XPCT oct(ir[10-15]) : 077
