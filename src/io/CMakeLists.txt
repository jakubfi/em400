ADD_library(io
	io.c
	cmem.c cmem_m9425.c
	cchar.c cchar_term.c
	multix.c multix_winch.c multix_floppy.c multix_puncher.c multix_punchreader.c multix_term.c
	plix.c
	e4image.c
	term.c
)

# POSIX: term.c needs IEEE Std 1003.1g-2000 pselect(2)
# POSIX: term.c needs IEEE Std 1003.1b-1993 nanosleep(2) and (struct timespec)
# POSIX: term.c needs X/Open 5 fsync(2)
set_property(
	SOURCE term.c
	APPEND
	PROPERTY COMPILE_DEFINITIONS
	_XOPEN_SOURCE=600
)

# vim: tabstop=4
