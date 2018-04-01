add_library(mx
	mx.c cmds.c irq.c event.c line.c
	proto_winchester.c proto_floppy.c proto_tape.c
	proto_puncher.c proto_punchrd.c	proto_terminal.c
	proto_som_puncher.c proto_som_punchrd.c proto_som_terminal.c
)

target_link_libraries(mx
	dev utils
)

# vim: tabstop=4
