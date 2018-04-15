
        .ifndef __IOTESTER_INC__
        .const __IOTESTER_INC__ 1

;                         E cccc SSSSSS nnnn C
	.const	CMD_ECO 0b1_0000_000000_0000_0 ; FETCH
	.const	CMD_ISP	0b0_0001_000000_0000_0 ; FETCH
	.const	CMD_RND	0b0_0010_000000_0000_0 ; FETCH
	.const	CMD_ANS	0b0_0011_000000_0000_0 ; FETCH
	.const	CMD_WNB	0b0_0001_000000_0000_0 ; SEND
	.const	CMD_WAM	0b0_0010_000000_0000_0 ; SEND
	.const	CMD_WAB	0b0_0011_000000_0000_0 ; SEND
	.const	CMD_WM	0b0_0100_000000_0000_0 ; SEND
	.const	CMD_RM	0b0_0101_000000_0000_0 ; SEND
	.const	CMD_WMM	0b0_0110_000000_0000_0 ; SEND
	.const	CMD_RMM	0b0_0111_000000_0000_0 ; SEND
	.const	CMD_IRQ	0b0_1000_000000_0000_0 ; SEND
	.const	CMD_PA	0b0_1001_000000_0000_0 ; SEND
	.const	PGM_END -1

iotester_chan:
	.word	14

; ---------------------------------------------------
; r7: out: interrupt specification
iotester_iv:
	md	[STACKP]
	lw	r7, [-4]	; load IC
	awt	r7, 1		; increment by one to skip the HLT/NOP magic
	md	[STACKP]
	rw	r7, -4		; store new IC in the vector
	md	[STACKP]
	lw	r7, [-1]	; load interrupt specification
	lip

; ---------------------------------------------------
; r1 - arg
; r2 - cmd
__it_cmd__:
	.res	1
	md	[iotester_chan]
iten:	ou	r1, r2
	.word	itno, iten, itok, itpe
itno:	hlt	060
itpe:	hlt	061
itok:	hlt
	nop
	uj	[__it_cmd__]

; ---------------------------------------------------
; r1: in: channel number to talk to
iotester_setchan:
	.res	1
	slz	r1
	rw	r1, iotester_chan
	uj	[iotester_setchan]

; ---------------------------------------------------
; r1: in: AM
iotester_wam:
	.res	1
	lw	r2, CMD_WAM
	lj	__it_cmd__
	uj	[iotester_wam]

; ---------------------------------------------------
; r1: in: AB
iotester_wab:
	.res	1
	lw	r2, CMD_WAB
	lj	__it_cmd__
	uj	[iotester_wab]

; ---------------------------------------------------
; r1: in: word count
iotester_rm:
	.res	1
	lw	r2, CMD_RM
	lj	__it_cmd__
	uj	[iotester_rm]

; ---------------------------------------------------
; r1: in: word count
iotester_wm:
	.res	1
	lw	r2, CMD_WM
	lj	__it_cmd__
	uj	[iotester_wm]

; ---------------------------------------------------
; r1: in: word count
iotester_rmm:
	.res	1
	lw	r2, CMD_RMM
	lj	__it_cmd__
	uj	[iotester_rmm]

; ---------------------------------------------------
; r1: in: word count
iotester_wmm:
	.res	1
	lw	r2, CMD_WMM
	lj	__it_cmd__
	uj	[iotester_wmm]

; ---------------------------------------------------
; r1: in: requested interrupt specification
iotester_irq:
	.res	1
	lw	r2, CMD_IRQ
	lj	__it_cmd__
	uj	[iotester_irq]

; ---------------------------------------------------
; r1: out: random number
iotester_rnd:
	.res	1
	md	[iotester_chan]
	in	r1, CMD_RND
	.word	rnderr, iotester_rnd+1, rndok, rnderr
rnderr:	hlt	062
rndok:	uj	[iotester_rnd]

; ---------------------------------------------------
; r2: in: command bits (modified in the procedure)
; r1: out: response
iotester_eco:
	.res	1
	or	r2, [iotester_chan]
	or	r2, CMD_ECO
ecoen:	in	r1, r2
	.word	econo, ecoen, ecook, ecope
econo:	hlt	063
ecope:	hlt	064
ecook:	uj	[iotester_eco]

; ---------------------------------------------------
iotester_pa:
	.res	1
paen:	ou	r1, CMD_PA
	.word	pano, paen, paok, pape
pano:	hlt	065
pape:	hlt	066
paok:	hlt
	nop
	uj	[iotester_pa]

; ---------------------------------------------------
; r4 - program address
xregs:	.res	7
exec:	.res	1
xloop:	rf	xregs
	lw	r1, [r4]
	cw	r1, PGM_END
	je	xdone
	lw	r2, [r4+1]
	slz	r1
	jys	xenin
xenou:	ou	r2, r1
	.word	xno, xenou, xok, xpe
xenin:	in	r2, r1
	.word	xno, xenin, xok, xpe
xpe:	hlt	041
xno:	hlt	042
xok:	awt	r4, 2
	ujs	xloop
xdone:	lf	xregs
	uj	[exec]

	.endif

; vim: tabstop=8 shiftwidth=8 autoindent syntax=emas
