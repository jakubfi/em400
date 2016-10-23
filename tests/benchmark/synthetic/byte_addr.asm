	lwt r5, -60

loop:	lb r1, r7
	lb r1, r7+1
	cb r1, r7
	cb r1, r7+1
	rb r1, r7
	rb r1, r7+1
	lb r1, r7
	lb r1, r7+1
	cb r1, r7
	cb r1, r7+1
	rb r1, r7
	rb r1, r7+1
	lb r1, r7
	lb r1, r7+1
	cb r1, r7
	cb r1, r7+1
	rb r1, r7
	rb r1, r7+1
	lb r1, r7
	lb r1, r7+1
	cb r1, r7
	cb r1, r7+1
	rb r1, r7
	rb r1, r7+1
	lb r1, r7
	lb r1, r7+1
	cb r1, r7
	cb r1, r7+1
	rb r1, r7
	rb r1, r7+1
	lb r1, r7
	lb r1, r7+1
	cb r1, r7
	cb r1, r7+1
	rb r1, r7
	rb r1, r7+1

	irb r5, loop
	ujs loop
	hlt 077
