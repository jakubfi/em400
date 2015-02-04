add_library(io
	io.c chan.c
#	cmem.c cmem_m9425.c
	cchar.c cchar_term.c
	dev.c dev_winch.c
	mx.c mx_intr.c mx_cmd.c
	mx_proto.c mx_proto_winch.c
)

add_subdirectory(dev)

# vim: tabstop=4
