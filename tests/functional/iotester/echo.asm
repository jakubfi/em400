; OPTS -c configs/iotester.cfg

; Test if I/O command bits come through as they should

	.cpu	mera400

	.include hw.inc

	uj	start

	.org	OS_MEM_BEG

	.include iotester.inc

; ------------------------------------------------
; r4 - current channel number
; r3 - current tested command bit
; r2 - requested response (modified by iotester_eco)
; r1 - actual response
start:
	lw	r4, 0			; start with channel 0
nxtch:
	lw	r3, 0b1_0000_000000_0000_0	; start with no bit set (except CMD_ECO bit)
nxtb:
	lw	r1, r4			; load channel number
	lj	iotester_setchan
	lw	r2, r3			; load current test bit
	lj	iotester_eco

	cl	r1, r2			; response = request ?
	jn	errv
	srz	r3			; test next bit
	cw	r3, 0b0_0000_000000_1000_0	; was it the last bit?
	jn	nxtb
	awt	r4, 1			; select next channel
	cw	r4, 16			; is it the last channel?
	jn	nxtch
	hlt	077

err:	hlt	040
errv:	hlt	041

; XPCT ir : 0xec3f
