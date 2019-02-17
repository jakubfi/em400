; OPTS -c configs/mod.cfg

; 17th bit for byte addressing is not set when
; negative pre-mod causes carry=1 when added to rC+rB

	.cpu	mx16

	.include cpu.inc
	.include io.inc

	.const	magic 0xfeba
	.const	addr 0x200
	.const	seg 4

	uj	start

mask:	.word	IMASK_NOMEM
zeroreg:.res	7, 0

nomem_proc:
	hlt	040

	.org	OS_START

start:	la	zeroreg
	lw	r1, stack
	rw	r1, STACKP
	lw	r1, nomem_proc
	rw	r1, INTV_NOMEM

	lw	r1, seg\3 | 0\15
	ou	r1, 3\10 | 0\14 | MEM_CFG
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

stack:

; XPCT r2 : 0x00ba
; XPCT ir : 0xec3f
