; OPTS -c configs/minimal.ini

; MERA-400 system memory block (8k) test
; The original "T8K" test provided by the manufacturer.
; Disassembled and reformatted for better readability.
;
; If 'EM400' constant is set, assembly is targeted for em400 emulation tests.
; Otherwise output is identical to the TP binary and targeted for h/w.
;
; If 'FIX_PARITY_ERR' constant is set, parity interrupt is unmasked
; after the software memory configuration test, to allow T8K run
; on silicon memory too.

	.cpu	mera400

; use EM400 compatibile hlt codes if running in an emulator
.ifdef EM400
	.const	ERR_CODE 040
.else
	.const	ERR_CODE 0
.endif

	.const	INTV_PARITY 0x41
	.const	STACKP 0x61
	.const	RZ_NOMEM_INT 1\2

	uj	start

; ------------------------------------------------------------------------
; ------------------------------------------------------------------------
; ------------------------------------------------------------------------

	.org	0x100
start:

	lw	r1, int_parity
	rw	r1, INTV_PARITY

	lwt	r1, 1
	rw	r1, phase

	mcl
.ifndef FIX_PARITY_ERR
	im	int_mask
.endif
	lw	r6, 0x1000
	rz	reg_sr
	rw	r6, first_frame_in_module

; --- Test Softwareowego Podziału Pamięci --------------------------------

tspp:
	rj	r7, set_flag_7
	mb	reg_sr
	tw	r4, r6
	rj	r7, check_for_nomem

	lw	r7, [reg_sr]
	cwt	r7, 15
	jes	.last_module
	cw	r6, 0xf000
	jes	.last_frame

.next_frame:
	aw	r6, 0x1000
	ujs	tspp

.last_frame:
	ib	reg_sr
	lwt	r6, 0
	ujs	tspp

.last_module:
	cw	r6, 0xe000
	jes	trg
	ujs	.next_frame

; --- Test Rozkazów Grupowych --------------------------------------------

trg:

.ifdef FIX_PARITY_ERR
	im	int_mask
.endif

	mb	block_addr
	lw	r1, freemem_start
	rw	r1, test_addr_start
	lw	r1, 0x1fff
	rw	r1, test_addr_end
	lw	r4, [test_addr_start]
	lwt	r1, -1
	lwt	r2, -1
	rd	pattern

	lw	r1, r0 rd r4
	lw	r2, r0 rd [r4]
	lw	r3, r0 rd r0
	lw	r5, r0 rd [r0]
	rw	r1, test_opcode
	lwt	r6, -20		; 20 repetitions
	lj	trg_rdpd

	lw	r1, r0 pd r4
	lw	r2, r0 pd [r4]
	lw	r3, r0 pd r0
	lw	r5, r0 pd [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_rdpd

	lw	r4, [test_addr_start]

	lw	r1, r0 ld r4
	lw	r2, r0 ld [r4]
	lw	r3, r0 ld r0
	lw	r5, r0 ld [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_ldtd

	lw	r1, r0 td r4
	lw	r2, r0 td [r4]
	lw	r3, r0 td r0
	lw	r5, r0 td [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_ldtd

; ------------------------------------------------------------------------
; if key 7 on the control panel is up, skip this part
	rky	r7
	bn	r7, 1\7		; key 7
	uj	.skip_dword_arith
	lw	r4, [test_addr_start]

	lw	r1, r0 ad r4
	lw	r2, r0 ad [r4]
	lw	r3, r0 ad r0
	lw	r5, r0 ad [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_ldtd

	lw	r1, r0 sd r4
	lw	r2, r0 sd [r4]
	lw	r3, r0 sd r0
	lw	r5, r0 sd [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_ldtd

	lw	r4, [test_addr_start]

	lw	r1, r0 mw r4
	lw	r2, r0 mw [r4]
	lw	r3, r0 mw r0
	lw	r5, r0 mw [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_ldtd

	lwt	r1, 0
	rw	r1, pattern
	lw	r1, r0 dw r4
	lw	r2, r0 dw [r4]
	lw	r3, r0 dw r0
	lw	r5, r0 dw [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_ldtd

.skip_dword_arith:
	lw	r1, 0x6666
	lw	r2, 0x3333
	lw	r3, 0x3301
	rd	pattern
	rw	r3, pattern+2

	lw	r1, r0 rf r4
	lw	r2, r0 rf [r4]
	lw	r3, r0 rf r0
	lw	r5, r0 rf [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_rfpf_rlpl

	lw	r1, r0 pf r4
	lw	r2, r0 pf [r4]
	lw	r3, r0 pf r0
	lw	r5, r0 pf [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_rfpf_rlpl

	lwt	r1, 0
	lwt	r2, 0
	lwt	r3, 0
	rf	pattern
	lw	r1, 0x6666
	lw	r2, 0x3333
	lw	r3, 0x3301
	rf	pattern+4

	lw	r1, r0 rl r4
	lw	r2, r0 rl [r4]
	lw	r3, r0 rl r0
	lw	r5, r0 rl [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_rfpf_rlpl

	lw	r1, r0 pl r4
	lw	r2, r0 pl [r4]
	lw	r3, r0 pl r0
	lw	r5, r0 pl [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_rfpf_rlpl

	md	[test_addr_start]
	lw	r4, 0x0140

	lw	r1, r0 ll r4
	lw	r2, r0 ll [r4]
	lw	r3, r0 ll r0
	lw	r5, r0 ll [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_lltl_lftf

	lw	r1, r0 tl r4
	lw	r2, r0 tl [r4]
	lw	r3, r0 tl r0
	lw	r5, r0 tl [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_lltl_lftf

	lw	r1, 0x6666
	lw	r2, 0x3333
	lw	r3, 0x3301
	rd	pattern
	rw	r3, pattern+2
	rz	pattern+4
	rz	pattern+5
	rz	pattern+6

	lw	r1, r0 lf r4
	lw	r2, r0 lf [r4]
	lw	r3, r0 lf r0
	lw	r5, r0 lf [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_lltl_lftf

	lw	r1, r0 tf r4
	lw	r2, r0 tf [r4]
	lw	r3, r0 tf r0
	lw	r5, r0 tf [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_lltl_lftf

; ------------------------------------------------------------------------
; if key 7 on the control panel is up, skip this part
	rky	r7
	bn	r7, 1\7		; key 7
	uj	.skip_float_arith
	md	[test_addr_start]
	lw	r4, 0x0140
	ib	pattern+2

	lw	r1, r0 af r4
	lw	r2, r0 af [r4]
	lw	r3, r0 af r0
	lw	r5, r0 af [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_lltl_lftf

	lw	r1, 0x3030
	rw	r1, pattern

	lw	r1, r0 sf r4
	lw	r2, r0 sf [r4]
	lw	r3, r0 sf r0
	lw	r5, r0 sf [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_lltl_lftf

	lw	r1, 0x51eb
	lw	r2, 0x3333
	lw	r3, 0x4702
	rd	pattern
	rw	r3, pattern+2

	lw	r1, r0 mf r4
	lw	r2, r0 mf [r4]
	lw	r3, r0 mf r0
	lw	r5, r0 mf [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_lltl_lftf

	lw	r1, 0x4000
	lw	r2, 0
	lw	r3, 1
	rd	pattern
	lw	r3, 0x02d4

	lw	r1, r0 df r4
	lw	r2, r0 df [r4]
	lw	r3, r0 df r0
	lw	r5, r0 df [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_lltl_lftf

.skip_float_arith:
	lw	r1, r0 ra r4
	lw	r2, r0 ra [r4]
	lw	r3, r0 ra r0
	lw	r5, r0 ra [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_rapa

	lw	r1, r0 pa r4
	lw	r2, r0 pa [r4]
	lw	r3, r0 pa r0
	lw	r5, r0 pa [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_rapa

	md	[test_addr_start]
	lw	r4, 0x0500

	lw	r1, r0 la r4
	lw	r2, r0 la [r4]
	lw	r3, r0 la r0
	lw	r5, r0 la [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_lata

	lw	r1, r0 ta r4
	lw	r2, r0 ta [r4]
	lw	r3, r0 ta r0
	lw	r5, r0 ta [r0]
	rw	r1, test_opcode
	lwt	r6, -20
	lj	trg_lata

	ujs	trg_fill_pftf

pattern:
	.res	7

; ------------------------------------------------------------------------
trg_fill_pftf:
	lwt	r1, 0
	rw	r1, block_addr2
	lw	r1, r0 pf r4
	lwt	r6, -20
	rw	r1, test_opcode
	lw	r4, freemem_start
.loop:
	lwt	r1, -1
	lwt	r2, -1
	lwt	r3, -1
	mb	block_addr2
	pf	r4
	lwt	r1, 0
	lwt	r2, 0
	lwt	r3, 0
	tf	r4
	mb	block_addr
	ki	int_reg
	cwt	r1, -1
	jn	.err
	cwt	r2, -1
	jn	.err
	cwt	r3, -1
	jn	.err
.continue:
	awt	r4, 3
	cw	r4, 0x1ffd
	jgs	trg_fill_pltl
	ujs	.loop
.err:
	mb	block_addr
	hlt	ERR_CODE	; @ 0x0301
	rz	int_reg
	fi	int_reg
	lw	r4, r2
	ujs	.continue

; ------------------------------------------------------------------------
trg_fill_pltl:
	lw	r1, r0 pl r4
	lwt	r6, -20
	rw	r1, test_opcode
	lw	r4, freemem_start
.loop:
	lwt	r5, -1
	lwt	r6, -1
	lwt	r7, -1
	mb	block_addr2
	pl	r4
	lwt	r5, 0
	lwt	r6, 0
	lwt	r7, 0
	tl	r4
	mb	block_addr
	ki	int_reg
	cwt	r5, -1
	jn	.err
	cwt	r6, -1
	jn	.err
	cwt	r7, -1
	jn	.err
.continue:
	awt	r4, 3
	cw	r4, 0x1ffd
	jgs	trg_fill_pata
	ujs	.loop
.err:
	mb	block_addr
	hlt	ERR_CODE	; @ 0x032d
	rz	int_reg
	fi	int_reg
	lw	r4, r2
	ujs	.continue

; ------------------------------------------------------------------------
trg_fill_pata:
	mb	block_addr
	lw	r1, r0 pa r4
	lwt	r6, -20
	rw	r1, test_opcode
	lw	r4, freemem_start
.loop:
	lwt	r1, -1
	lwt	r2, -1
	lwt	r3, -1
	lwt	r5, -1
	lwt	r6, -1
	lwt	r7, -1
	rw	r4, tmp_r4
	mb	block_addr2
	pa	r4
	lwt	r1, 0
	lwt	r2, 0
	lwt	r3, 0
	lwt	r5, 0
	lwt	r6, 0
	lwt	r7, 0
	ta	r4
	mb	block_addr
	ki	int_reg
	cwt	r1, -1
	jn	.err
	cwt	r2, -1
	jn	.err
	cwt	r3, -1
	jn	.err
	cw	r4, [tmp_r4]
	jn	.err
	cwt	r5, -1
	jn	.err
	cwt	r6, -1
	jn	.err
	cwt	r7, -1
	jn	.err
.continue:
	awt	r4, 7
	cw	r4, 0x1ff9
	jgs	.fin
	ujs	.loop
.err:
	mb	block_addr
	hlt	ERR_CODE	; @ 0x0370
	rz	int_reg
	fi	int_reg
	ujs	.continue
.fin:
	rz	block_addr2

; --- Test Zapisu i Odczytu Zer ------------------------------------------

tzoz:
	rky	r7
	bn	r7, 1\8		; key 8
	ujs	tzoj
	lwt	r2, 3
	rw	r2, test_num
	lwt	r2, 0		; test pattern
	lj	run_pattern_test

; --- Test Zapisu i Odczytu Jedynek --------------------------------------

	rky	r7
tzoj:	bn	r7, 1\9		; key 9
	ujs	tpz
	lwt	r2, 4
	rw	r2, test_num
	lwt	r2, -1		; test pattern
	lj	run_pattern_test

; --- Test Pływającego Zera ----------------------------------------------

	rky	r7
tpz:	bn	r7, 1\10	; key 10
	ujs	tpj
	lwt	r2, 5
	rw	r2, test_num
	lw	r2, 0x7fff	; test pattern
	lj	test_all_shifts

; --- Test Pływającej Jedynki --------------------------------------------

	rky	r7
tpj:	bn	r7, 1\11	; key 11
	ujs	tua
	lwt	r2, 6
	rw	r2, test_num
	lw	r2, 0x8000	; test pattern
	lj	test_all_shifts

; --- Test Układu Adresacji ----------------------------------------------

	rky	r7
tua:	bn	r7, 1\12	; key 12
	ujs	tdsd
	lwt	r1, 7
	rw	r1, test_num
	lw	r1, [test_addr_start]

	lwt	r2, 0
.zero_loop:
	rj	r7, store_and_check_if_fin
	.word	.zero_fin
	ujs	.zero_loop
.zero_fin:
	lw	r1, [test_addr_start]
.addr_loop:
	lw	r2, r1
	rj	r7, store_and_check_if_fin
	.word	read_fw
	ujs	.addr_loop

read_fw:
	lw	r1, [test_addr_start]
.loop:
	lw	r2, r1
	cw	r2, [r1]
	ki	int_reg
	jes	.continue
	lj	err_halt
.continue:
	cw	r1, [test_addr_end]
	jes	read_bw
	awt	r1, 1
	ujs	.loop

read_bw:
	lw	r1, [test_addr_end]
.loop:
	lw	r2, r1
	cw	r2, [r1]
	ki	int_reg
	jes	.continue
	lj	err_halt
.continue:
	cw	r1, [test_addr_start]
	jes	tdsd_start
	awt	r1, -1
	ujs	.loop

; --- Test Dodatnich Szumów Delta ----------------------------------------

tdsd_start:
	rky	r7
tdsd:
	bn	r7, 1\13	; key 13
	ujs	tusd
	lwt	r1, 8
	rw	r1, test_num
	lw	r1, [test_addr_start]
.loop:
	rj	r7, xor_addr_bits_1_7
	rj	r7, store_and_check_if_fin
	.word	.fin
	ujs	.loop
.fin:	lw	r1, [test_addr_start]
.loop2:
	rj	r7, xor_addr_bits_1_7
	rj	r7, check_2stores_and_check_if_fin
	.word	tusd_start
	ujs	.loop2

; --- Test Ujemnych Szumów Delta -----------------------------------------

tusd_start:
	rky	r7
tusd:
	bn	r7, 1\14	; key 14
	ujs	all_tests_done
	lwt	r1, 9
	rw	r1, test_num
	lw	r1, [test_addr_start]
.loop:
	rj	r7, xor_addr_bits_1_7
	ngl	r2
	rj	r7, store_and_check_if_fin
	.word	.fin
	ujs	.loop
.fin:	lw	r1, [test_addr_start]
.loop2:
	rj	r7, xor_addr_bits_1_7
	ngl	r2
	rj	r7, check_2stores_and_check_if_fin
	.word	all_tests_done
	ujs	.loop2

; ------------------------------------------------------------------------

all_tests_done:
	lw	r1, [phase]
	cwt	r1, 1
	jes	test_first_64w
.ifdef EM400
	hlt	077	; finish and indicate no error if running in EM400 emulator
	.res	1	; padding for consistent addressing
.else
	uj	start
.endif

test_first_64w:
	rz	phase
	lw	r1, 2
	rw	r1, test_addr_start
	lw	r1, 0x64
	rw	r1, test_addr_end
	uj	tzoz

; ------------------------------------------------------------------------
int_parity:
	ra	reg_r1
	lw	r1, [STACKP]
	awt	r1, -4
	lf	r1
	hlt	ERR_CODE	; @ 0x041b
	la	reg_r1
	lip

; ------------------------------------------------------------------------
set_flag_7:
	cw	r6, [first_frame_in_module]
	jes	.first_frame
.not_first_frame_in_first_module:
	lwt	r0, ?7
	uj	r7
.first_frame:
	lw	r5, [reg_sr]
	cw	r5, [block_addr]
	jes	.first_module
	ujs	.not_first_frame_in_first_module
.first_module:
	lwt	r0, 0
	uj	r7

; ------------------------------------------------------------------------
check_for_nomem:
	ki	int_reg
	lw	r5, [int_reg]
	brc	?7
	ujs	.non_os_mem
	bb	r5, RZ_NOMEM_INT	; for OS segment 0 page 0x1000
	ujs	.ok			; it is expexted that no interrupt is set (page should be configured)
	ujs	.fail
.non_os_mem:
	bb	r5, RZ_NOMEM_INT	; for all other pages in all segments
	ujs	.fail			; the interrupt is expected to be set (page is not configured)
.ok:
	lwt	r5, 0
	rw	r5, int_reg
	fi	int_reg
	uj	r7
.fail:
	lw	r5, [reg_sr]
	hlt	ERR_CODE	; @ 0x0441
	ujs	.ok

; ------------------------------------------------------------------------
trg_rdpd:
	.res	1
	rws	r1, .register_indirect
	rws	r2, .register_double_indirect
	rws	r3, .indirect
	rws	r5, .double_indirect
	rj	r7, minus1_to_r1_r2
.register_indirect:
	rd	r4
	rj	r7, call_62f
	rj	r7, minus1_to_r1_r2
	irb	r6, .register_indirect
; --------
	lwt	r6, -20
	lws	r5, .register_double_indirect
	rw	r5, test_opcode
.register_double_indirect_loop:
	rj	r7, call_4c6
.register_double_indirect:
	rd	[r4]
	lw	r4, [r4]
	rj	r7, call_62f
	rj	r7, minus1_to_r1_r2
	irb	r6, .register_double_indirect_loop
; --------
	lwt	r6, -20
	lws	r5, .indirect
	rw	r5, test_opcode
.indirect_loop:
	rws	r4, .indirect_val
.indirect:
	rd	r0
.indirect_val:
	.word	0
	rj	r7, call_62f
	rj	r7, minus1_to_r1_r2
	irb	r6, .indirect_loop
; --------
	lwt	r6, -20
	lws	r5, .double_indirect
	rw	r5, test_opcode
.double_indirect_loop:
	rw	r4, addr_indirect
.double_indirect:
	rd	[addr_indirect]
	rj	r7, call_62f
	rj	r7, minus1_to_r1_r2
	irb	r6, .double_indirect_loop
	uj	[trg_rdpd]

; ------------------------------------------------------------------------
trg_ldtd:
	.res	1
	rws	r1, word_47f
	rws	r2, word_48b
	rws	r3, word_497
	rws	r5, word_4a4
jmp_47d:
	rj	r7, call_4ab
word_47f:
	ld	r4
	rj	r7, call_64d
	irb	r6, jmp_47d
; --------
	lwt	r6, -20
	lws	r5, word_48b
	rw	r5, test_opcode
jmp_487:
	rj	r7, call_4c6
	rj	r7, call_4ab
word_48b:
	ld	[r4]
	lw	r4, [r4]
	rj	r7, call_64d
	irb	r6, jmp_487
; --------
	lwt	r6, -20
	lws	r5, word_497
	rw	r5, test_opcode
jmp_494:
	rws	r4, word_498
	rj	r7, call_4ab
word_497:
	ld	r0
word_498:
	.word 0
	rj	r7, call_64d
	irb	r6, jmp_494
; --------
	lwt	r6, -20
	lws	r5, word_4a4
	rw	r5, test_opcode
jmp_4a0:
	rw	r4, addr_indirect
	rj	r7, call_4ab
word_4a4:
	ld	[addr_indirect]
	rj	r7, call_64d
	irb	r6, jmp_4a0
	uj	[trg_ldtd]

; ------------------------------------------------------------------------
call_4ab:
	lw	r1, [test_opcode]
	cwt	r1, 0
	jl	r7
	bc	r1, 0x00c0
	ujs	jmp_4c0
	bc	r1, 0x0080
	ujs	jmp_4be
	bc	r1, 0x0040
	ujs	jmp_4bb
	lwt	r1, 0
	lwt	r2, 0
	uj	r7
jmp_4bb:
	lwt	r1, -1
	lwt	r2, -2
	uj	r7
jmp_4be:
	lwt	r2, 1
	uj	r7
jmp_4c0:
	lwt	r1, 0
	lwt	r2, 1
	uj	r7

; ------------------------------------------------------------------------
minus1_to_r1_r2:
	lwt	r1, -1
	lwt	r2, -1
	uj	r7

; ------------------------------------------------------------------------
call_4c6:
	rw	r4, addr_indirect
	lw	r4, 0x072d
	uj	r7

; ------------------------------------------------------------------------
trg_rfpf_rlpl:
	.res	1
	rws	r1, word_4d4
	rws	r2, word_4e4
	rws	r3, word_4f4
	rws	r5, word_505
jmp_4d0:
	rw	r6, repetitions
	lj	word_50e
word_4d4:
	rf	r4
	rj	r7, call_653
	lw	r6, [repetitions]
	irb	r6, jmp_4d0
	lwt	r6, -20
	lws	r5, word_4e4
	rw	r5, test_opcode
jmp_4de:
	rj	r7, call_4c6
	rw	r6, repetitions
	lj	word_50e
word_4e4:
	rf	[r4]
	lw	r4, [r4]
	rj	r7, call_653
	lw	r6, [repetitions]
	irb	r6, jmp_4de
	lwt	r6, -20
	lws	r5, word_4f4
	rw	r5, test_opcode
jmp_4ef:
	rws	r4, word_4f5
	rw	r6, repetitions
	lj	word_50e
word_4f4:
	rf	r0
word_4f5:
	.word 0
	rj	r7, call_653
	lw	r6, [repetitions]
	irb	r6, jmp_4ef
	lwt	r6, -20
	lws	r5, word_505
	rw	r5, test_opcode
jmp_4ff:
	rw	r4, addr_indirect
	rw	r6, repetitions
	lj	word_50e
word_505:
	rf	[addr_indirect]
	rj	r7, call_653
	lw	r6, [repetitions]
	irb	r6, jmp_4ff
	uj	[trg_rfpf_rlpl]

; ------------------------------------------------------------------------
word_50e:
	.word 0x0000
	lw	r1, [test_opcode]
	cwt	r1, 0
	jls	jmp_51b
jmp_513:
	lw	r1, 0x6666
	lw	r2, 0x3333
	lw	r3, 0x3301
	uj	[word_50e]
jmp_51b:
	bb	r1, 0x00c0
	ujs	jmp_513
	lw	r5, 0x6666
	lw	r6, 0x3333
	lw	r7, 0x3301
	uj	[word_50e]

; ------------------------------------------------------------------------
trg_lltl_lftf:
	.word 0x0000
	rws	r1, word_52f
	rws	r2, word_53d
	rws	r3, word_54b
	rws	r5, word_55a
jmp_52b:
	rw	r6, repetitions
	lj	word_561
word_52f:
	ll	r4
	lj	word_56f
	irb	r6, jmp_52b
	lwt	r6, -20
	lws	r5, word_53d
	rw	r5, test_opcode
jmp_537:
	rj	r7, call_4c6
	rw	r6, repetitions
	lj	word_561
word_53d:
	ll	[r4]
	lw	r4, [r4]
	lj	word_56f
	irb	r6, jmp_537
	lwt	r6, -20
	lws	r5, word_54b
	rw	r5, test_opcode
jmp_546:
	rws	r4, word_54c
	rw	r6, repetitions
	lj	word_561
word_54b:
	ll	r0
word_54c:
	.word 0
	lj	word_56f
	irb	r6, jmp_546
	lwt	r6, -20
	lws	r5, word_55a
	rw	r5, test_opcode
jmp_554:
	rw	r4, addr_indirect
	rw	r6, repetitions
	lj	word_561
word_55a:
	ll	[addr_indirect]
	lj	word_56f
	irb	r6, jmp_554
	uj	[trg_lltl_lftf]

; ------------------------------------------------------------------------
word_561:
	.res	1
	lw	r1, [test_opcode]
	cwt	r1, 0
	jl	[word_561]
	lw	r1, 0x6666
	lw	r2, 0x3333
	lw	r3, 0x3301
	uj	[word_561]

; ------------------------------------------------------------------------
word_56f:
	.res	1
	rws	r4, word_583
	lw	r4, [test_opcode]
	cwt	r4, 0
	jls	jmp_57c
jmp_575:
	lws	r4, word_583
	rj	r7, call_675
	lw	r6, [repetitions]
	uj	[word_56f]
jmp_57c:
	bb	r4, 0x00c0
	ujs	jmp_575
	lw	r1, r5
	lw	r2, r6
	lw	r3, r7
	ujs	jmp_575
word_583:
	.word 0x0000

; ------------------------------------------------------------------------
trg_rapa:
	.res	1
	rws	r1, word_58b
	rws	r2, word_597
	rws	r3, word_5a3
	rws	r5, word_5b0
jmp_589:
	rj	r7, store_regs
word_58b:
	ra	r4
	rj	r7, call_5b7
	irb	r6, jmp_589
	lwt	r6, -20
	lws	r5, word_597
	rw	r5, test_opcode
jmp_593:
	rj	r7, call_4c6
	rj	r7, store_regs
word_597:
	ra	[r4]
	lw	r4, [r4]
	rj	r7, call_5b7
	irb	r6, jmp_593
	lwt	r6, -20
	lws	r5, word_5a3
	rw	r5, test_opcode
jmp_5a0:
	rws	r4, word_5a4
	rj	r7, store_regs
word_5a3:
	ra	r0
word_5a4:
	.word 0
	rj	r7, call_5b7
	irb	r6, jmp_5a0
	lwt	r6, -20
	lws	r5, word_5b0
	rw	r5, test_opcode
jmp_5ac:
	rw	r4, addr_indirect
	rj	r7, store_regs
word_5b0:
	ra	[addr_indirect]
	rj	r7, call_5b7
	irb	r6, jmp_5ac
	uj	[trg_rapa]

; ------------------------------------------------------------------------
call_5b7:
	ki	int_reg
	lwt	r2, 0
jmp_5ba:
	lw	r1, [r4+r2]
	cw	r1, [0x0618+r2]
	jn	jmp_5c3
	awt	r2, 1
	cwt	r2, 7
	jes	jmp_5c4
	ujs	jmp_5ba
jmp_5c3:
	hlt	ERR_CODE	; @ 0x05c3
jmp_5c4:
	awt	r4, 7
	uj	r7

; ------------------------------------------------------------------------
trg_lata:
	.res	1
	rws	r1, word_5cf
	rws	r2, word_5df
	rws	r3, word_5ef
	rws	r5, word_600
jmp_5cb:
	rw	r4, word_583
	rw	r6, repetitions
word_5cf:
	la	r4
	lj	word_620
	rj	r7, call_5b7
	irb	r6, jmp_5cb
	lwt	r6, -20
	lws	r5, word_5df
	rw	r5, test_opcode
jmp_5d9:
	rj	r7, call_4c6
	rw	r4, word_583
	rw	r6, repetitions
word_5df:
	la	[r4]
	lj	word_620
	lw	r4, [r4]
	rj	r7, call_5b7
	irb	r6, jmp_5d9
	lwt	r6, -20
	lws	r5, word_5ef
	rw	r5, test_opcode
jmp_5ea:
	rws	r4, word_5f0
	rw	r4, word_583
	rw	r6, repetitions
word_5ef:
	la	r0
word_5f0:
	.word 0
	lj	word_620
	rj	r7, call_5b7
	irb	r6, jmp_5ea
	lwt	r6, -20
	lws	r5, word_600
	rw	r5, test_opcode
jmp_5fa:
	rw	r4, addr_indirect
	rw	r4, word_583
	rw	r6, repetitions
word_600:
	la	[addr_indirect]
	lj	word_620
	rj	r7, call_5b7
	irb	r6, jmp_5fa
	uj	[trg_lata]

; ------------------------------------------------------------------------
store_regs:
	rw	r1, reg_r1
	rw	r2, reg_r2
	rw	r3, reg_r3
	rw	r4, reg_r4
	rw	r5, reg_r5
	rw	r6, reg_r6
	rw	r7, reg_r7
	uj	r7

reg_r1:		.word 0
reg_r2:		.word 0
reg_r3:		.word 0
reg_r4:		.word 0
reg_r5:		.word 0
reg_r6:		.word 0
reg_r7:		.word 0

; ------------------------------------------------------------------------
word_61f:
	.res	1
word_620:
	.res	1
	rw	r7, word_61f
	rj	r7, store_regs
	lw	r7, [word_61f]
	rw	r7, reg_r7
	lw	r4, [word_583]
	lw	r6, [repetitions]
	uj	[word_620]

; ------------------------------------------------------------------------
call_62f:
	lw	r1, [r4]
	md	1
	lw	r5, [r4]
	cwt	r1, -1
	jes	jmp_639
	lwt	r3, -1
	rw	r3, r4
jmp_637:
	hlt	ERR_CODE	; @ 0x0637
	ujs	jmp_640
jmp_639:
	cwt	r5, -1
	jes	jmp_640
	lwt	r3, -1
	md	1
	rw	r3, r4
	ujs	jmp_637
jmp_640:
	awt	r4, 2
	uj	r7

; ------------------------------------------------------------------------
jmp_642:
	lw	r5, r2
	cwt	r1, -1
	jes	jmp_646
	ujs	jmp_637
jmp_646:
	cwt	r5, -1
	jes	jmp_640
	ujs	jmp_637
jmp_649:
	lw	r5, r2
	cwt	r1, 0
	jes	jmp_646
	ujs	jmp_637
; ------------------------------------------------------------------------
call_64d:
	lw	r5, [test_opcode]
	bc	r5, 0x00c0
	ujs	jmp_649
	ujs	jmp_642

; ------------------------------------------------------------------------
call_653:
	lw	r1, [r4]
	md	1
	lw	r5, [r4]
	md	2
	lw	r3, [r4]
	cw	r1, 0x6666
	jes	jmp_663
	lw	r2, 0x6666
	rw	r2, r4
jmp_660:
	hlt	ERR_CODE	; @ 0x0660
jmp_661:
	awt	r4, 3
	uj	r7
jmp_663:
	cw	r5, 0x3333
	jes	jmp_66c
	lw	r2, 0x3333
	md	1
	rw	r2, r4
	ujs	jmp_660
jmp_66c:
	cw	r3, 0x3301
	jes	jmp_661
	lw	r2, 0x3301
	md	2
	rw	r2, r4
	ujs	jmp_660
; ------------------------------------------------------------------------
call_675:
	lw	r5, r2
	lw	r2, [test_opcode]
	cwt	r2, 0
	jls	jmp_6b9
	bb	r2, 0x0100
	ujs	jmp_660
	bn	r2, 0x00c0
	ujs	jmp_68d
	cw	r1, 0x6666
	jn	jmp_660
	cw	r5, 0x3333
	jn	jmp_660
	cw	r3, 0x3302
	jn	jmp_660
	ujs	jmp_661
jmp_68d:
	bb	r2, 0x00c0
	ujs	jmp_691
	ujs	jmp_6ad
jmp_691:
	bb	r2, 0x0080
	ujs	jmp_695
	ujs	jmp_69f
jmp_695:
	cwt	r1, 0
	jn	jmp_660
	cwt	r5, 0
	jn	jmp_660
	cwt	r3, 0
	jn	jmp_660
	ujs	jmp_661
jmp_69f:
	cw	r1, 0x51eb
	jn	jmp_660
	cw	r5, 0x3333
	jn	jmp_660
	cw	r3, 0x4702
	jn	jmp_660
	uj	jmp_661
jmp_6ad:
	cw	r1, 0x4000
	jn	jmp_660
	cwt	r5, 0
	jn	jmp_660
	cwt	r3, 1
	jn	jmp_660
	uj	jmp_661
jmp_6b9:
	cw	r1, 0x6666
	je	jmp_663
	lw	r2, 0x6666
	rw	r2, r4
	uj	jmp_660

; ------------------------------------------------------------------------
run_pattern_test:
	.res	1
	lw	r1, [test_addr_start]
.write_loop:
	rj	r7, store_and_check_if_fin
	.word	.start_read
	ujs	.write_loop
.start_read:
	lw	r1, [test_addr_start]
.read_loop:
	mb	block_addr2
	tw	r3, r1
	cw	r3, r2
	ki	int_reg
	jes	.continue
	lj	err_halt
.continue:
	cw	r1, [test_addr_end]
	je	[run_pattern_test]
	awt	r1, 1
	ujs	.read_loop

; ------------------------------------------------------------------------
test_all_shifts:
	.res	1
	lwt	r1, -16
	rw	r1, .counter
.loop:
	lj	run_pattern_test
	shc	r2, 1
	ib	.counter
	ujs	.loop
	uj	[test_all_shifts]
.counter:
	.word	-16

; ------------------------------------------------------------------------
store_and_check_if_fin:
	pw	r2, r1
	cw	r1, [test_addr_end]
	je	[r7]
	awt	r1, 1
	uj	1+r7

; ------------------------------------------------------------------------
xor_addr_bits_1_7:
	lw	r2, r1
	nr	r2, 0x0101
	cwt	r2, 0
	jes	.fin
	cw	r2, 0x0101
	jes	.equal
	lwt	r2, -1
.fin:
	uj	r7
.equal:
	lwt	r2, 0
	ujs	.fin

; ------------------------------------------------------------------------
check_2stores_and_check_if_fin:
	cw	r2, [r1]
	ki	int_reg
	jes	.ok
	lj	err_halt
.ok:
	ngl	r2
	rw	r2, r1
	cw	r2, [r1]
	ki	int_reg
	jes	.ok2
	lj	err_halt
.ok2:
	ngl	r2
	rw	r2, r1
	cw	r1, [test_addr_end]
	je	[r7]
	awt	r1, 1
	uj	1+r7

; ------------------------------------------------------------------------
err_halt:
	.res	1
	mb	block_addr2
	tw	r3, r1
	mb	block_addr
	hlt	ERR_CODE	; @ 0x0716
	rz	int_reg
	fi	int_reg
	uj	[err_halt]

tmp_r4:
	.word 0x0000
	.word 0x0000
	.word 0x0000
	.word 0x0000
	.word 0x0000
	.word 0x0000
	.word 0x0000
int_mask:	.word 0x8000
reg_sr:		.word 0x0000
first_frame_in_module:	.word 0x0000
block_addr:	.word 0x0000
block_addr2:	.word 0x0000

int_reg:	.word 0x0000	; @ 0x0729
test_opcode:	.word 0x0000
test_addr_start:.word 0x0000
test_addr_end:	.word 0x0000
addr_indirect:	.word 0x0000
test_num:	.word 0x0000

repetitions:	.word 0x0000
phase:		.word 0x0000
freemem_start:

; XPCT ir : 0xec3f
