ADD_library(mem
	mem.c mem_mega.c mem_elwro.c
)

# POSIX: mem.c needs X/Open 6 (pthread_spinlock_t)
set_property(
	SOURCE mem.c
	APPEND
	PROPERTY COMPILE_DEFINITIONS
	_XOPEN_SOURCE=600
)

# vim: tabstop=4
