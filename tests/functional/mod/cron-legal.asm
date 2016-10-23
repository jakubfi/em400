; OPTS -c configs/mod.cfg

; CRON is illegal for modified (MX-16) CPU

	.cpu mx16

	cron
	hlt 077

; XPCT rz[6] : 1
