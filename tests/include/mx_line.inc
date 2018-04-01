
	.ifndef __MX_LINE_INC__
	.const __MX_LINE_INC__ 1

	.const MX_LSTATE_NONE          0
	.const MX_LSTATE_SEND_START    1 << 0   ; send started
	.const MX_LSTATE_SEND_RUN      1 << 1   ; send running
	.const MX_LSTATE_RECV_START    1 << 2   ; receive started
	.const MX_LSTATE_RECV_RUN      1 << 3   ; receive running
	.const MX_LSTATE_CAN_STOP      1 << 4   ; stop after CAN (protocol 5)
	.const MX_LSTATE_STOP_CHAR     1 << 5   ; stop character received
	.const MX_LSTATE_PARITY_ERR    1 << 6   ; parity error
	.const MX_LSTATE_OPRQ          1 << 7   ; OPRQ
	.const MX_LSTATE_ATTACHED      1 << 8   ; line attached
	.const MX_LSTATE_TRANS         1 << 9   ; transmission active
	.const MX_LSTATE_5             1 << 10  ; (unused)
	.const MX_LSTATE_4             1 << 11  ; (unused)
	.const MX_LSTATE_TASK_XOFF     1 << 12  ; task suspended due to XOFF
	.const MX_LSTATE_TRANS_XOFF    1 << 13  ; transmission suspended due to XOFF
	.const MX_LSTATE_TRANS_LAST    1 << 14  ; transmission of a last fragment
	.const MX_LSTATE_0             1 << 15  ; (unused)

	.endif

; vim: tabstop=8 shiftwidth=8 autoindent syntax=emas
