	.cpu mera400

start:
	lwt	r2, 0
next:	lwt	r1, 0
	awt	r2, 1
	cwt	r2, 10
	jes	start
loop:	awt	r1, 1
	cwt	r1, 0
	jes	next
	ujs	loop

fin:	hlt	077
