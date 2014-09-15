; CONFIG configs/mod.cfg

; CRON is illegal for modified (MX-16) CPU

	.cpu mx16

	cron
	hlt 077

; XPCT int(rz[6]) : 1
