add_library(io
	io.c chan.c
	cmem.c cmem_m9425.c
	cchar.c cchar_term.c
	multix.c multix_winch.c multix_floppy.c multix_puncher.c multix_punchreader.c multix_term.c
)

add_subdirectory(dev)

# vim: tabstop=4
