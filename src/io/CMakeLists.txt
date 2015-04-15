add_library(io
	io.c chan.c
#	cmem.c cmem_m9425.c
	cchar.c cchar_term.c
	mx.c mx_cmds.c mx_ev.c mx_irq.c mx_line.c mx_timer.c
	mx_proto.c mx_proto_terminal.c mx_proto_puncher.c mx_proto_punchreader.c mx_proto_som_terminal.c mx_proto_som_puncher.c mx_proto_som_punchreader.c mx_proto_tape.c mx_proto_winchester.c mx_proto_floppy.c mx_proto_ttyitwl.c
)

add_subdirectory(dev)

# vim: tabstop=4
