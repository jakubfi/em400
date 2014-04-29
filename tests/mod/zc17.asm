; CONFIG configs/mod.conf

	.cpu mx16

	.equ int_nomem 0x40 + 2
	.equ stackp 0x61

	.equ magic1 0x4455
	.equ magic2 0xfeba
	.equ addr 0x200

	uj start

mask:	.word 0b0100000000000000
stack:	.res 16

nomem_proc:
	hlt 040

	.org 0x70

start:	lw r1, stack
	rw r1, stackp
	lw r1, nomem_proc
	rw r1, int_nomem

	lw r1, 8\3 + 0\15
	ou r1, 3\10 + 0\14 + 1
	.word err, err, ok, err

err:	hlt 040

ok:	im mask
	lw r1, magic1
	rw r1, addr
	lw r1, magic2
	rw r1, 1\0 + addr

	lw r7, 1\0 + addr

	md 1
	lb r2, r7+r7

	cron			; enable cpu modification

	md 1
	lb r3, r7+r7

	mcl			; mcl disables cpu modification

	md 1
	lb r4, r7+r7

	hlt 077

; XPCT hex(r2) : 0x0055
; XPCT hex(r3) : 0x00ba
; XPCT hex(r4) : 0x0055
; XPCT oct(ir[10-15]) : 077
