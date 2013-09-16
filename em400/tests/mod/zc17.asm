.prog "mod/zc17"

; CONFIG configs/mod.conf

	.equ int_nomem 0x40 + 2
	.equ stackp 0x61

	.equ magic1 0x4455
	.equ magic2 0xfeba
	.equ addr 0x200

	uj start

mask:	.data 0b0100000000000000
stack:	.res 16

nomem_proc:
	hlt 040

	.ic 0x70

start:	lw r1, stack
	rw r1, stackp
	lw r1, nomem_proc
	rw r1, int_nomem

	lw r1, 8\3 + 0\15
	ou r1, 3\10 + 0\14 + 1
	.data err, err, ok, err

err:	hlt 040

ok:	im mask
	lw r1, magic1
	rw r1, addr
	lw r1, magic2
	rw r1, 1\0 + addr
	md 1\0 + addr
	lb r2, 1\0 + addr + 1

	cron			; enable cpu modification

	md 1\0 + addr
	lb r3, 1\0 + addr + 1

	mcl			; mcl disables cpu modification

	md 1\0 + addr
	lb r4, 1\0 + addr + 1

	hlt 077

.finprog

; XPCT hex(r2) : 0x0055
; XPCT hex(r3) : 0x00ba
; XPCT hex(r4) : 0x0055
; XPCT oct(ir[10-15]) : 077
