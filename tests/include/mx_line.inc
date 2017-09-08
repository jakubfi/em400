
	.ifndef __MX_LINE_INC__
	.const __MX_LINE_INC__ 1

	.const MX_LSTATE_NONE		0
	.const MX_LSTATE_ATTACHED	1 << 0	; line attached
	.const MX_LSTATE_TRANS		1 << 1	; transmission active
	.const MX_LSTATE_2		1 << 2	; (unused)
	.const MX_LSTATE_3		1 << 3	; (unused)
	.const MX_LSTATE_TASK_XOFF	1 << 4	; task suspended due to XOFF
	.const MX_LSTATE_TRANS_XOFF	1 << 5	; transmission suspended due to XOFF
	.const MX_LSTATE_TRANS_LAST	1 << 6	; transmission of a last fragment
	.const MX_LSTATE_7		1 << 7	; (unused)
	.const MX_LSTATE_SEND_START	1 << 8	; send started
	.const MX_LSTATE_SEND_RUN	1 << 9	; send running
	.const MX_LSTATE_RECV_START	1 << 10	; receive started
	.const MX_LSTATE_RECV_RUN	1 << 11	; receive running
	.const MX_LSTATE_CAN_STOP	1 << 12	; stop after CAN (protocol 5)
	.const MX_LSTATE_STOP_CHAR	1 << 13	; stop character received
	.const MX_LSTATE_PARITY_ERR	1 << 14	; parity error
	.const MX_LSTATE_OPRQ		1 << 15	; OPRQ

	.endif

; vim: tabstop=8 shiftwidth=8 autoindent syntax=emas
