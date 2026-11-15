; DW misses some overflows with 32768 < |quotient| < 65536: no I_DIV_OF,
; r2 = quotient mod 2^16, r1 = exact remainder, Z and M from r2, V and C kept.
; Which overflows are missed depends on the operand bits, not only on the
; quotient. The last two rows are overflows that are detected.

	.include awp-dword.inc

operation:
	ld	r7+ARG1
	dw	r7+ARG2
	uj	r5

tests:
	; -32769 / 1
	.word	0, 0, 0
	.word	0xffff, 0x7fff
	.word	0x0001, 0
	.word	0x0000, 0x7fff

	; -32769 / -1
	.word	0, ?M, 0
	.word	0xffff, 0x7fff
	.word	0xffff, 0
	.word	0x0000, 0x8001

	; 2147483639 / -32768 = -65535, r 32759
	.word	0, 0, 0
	.word	0x7fff, 0xfff7
	.word	0x8000, 0
	.word	0x7ff7, 0x0001

	; 1102356795 / -18031 = -61136, r 13579
	.word	0, 0, 0
	.word	0x41b4, 0xa13b
	.word	0xb991, 0
	.word	0x350b, 0x1130

	; 1610563584 / 32767 = 49152
	.word	?Z|?M|?V|?C, ?M|?V|?C, 0
	.word	0x5fff, 0x4000
	.word	0x7fff, 0
	.word	0x0000, 0xc000

	; -2147418112 / -32768 = 65534
	.word	0, ?M, 0
	.word	0x8001, 0x0000
	.word	0x8000, 0
	.word	0x0000, 0xfffe

	; -1073714269 / -16384 = 65534, r -5213
	.word	?Z|?M|?V, ?M|?V, 0
	.word	0xc000, 0x6ba3
	.word	0xc000, 0
	.word	0xeba3, 0xfffe

	; 2147418113 / -32768 = -65534, r 1
	.word	?Z|?M|?V|?C, ?V|?C, 0
	.word	0x7fff, 0x0001
	.word	0x8000, 0
	.word	0x0001, 0x0002

	; detected: 8356349 / 255 = 32769, r 254
	.word	?Z|?M|?V|?C, ?Z|?M|?V|?C, I_DIV_OF
	.word	0x007f, 0x81fd
	.word	0x00ff, 0
	.word	0x007f, 0x81fd

	; detected: 536870912 / 16384 = 32768
	.word	0, 0, I_DIV_OF
	.word	0x2000, 0x0000
	.word	0x4000, 0
	.word	0x2000, 0x0000
fin:
