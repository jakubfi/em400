BISON_TARGET(parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.c)
FLEX_TARGET(scanner scanner.l ${CMAKE_CURRENT_BINARY_DIR}/scanner.c COMPILE_FLAGS "--never-interactive")
ADD_FLEX_BISON_DEPENDENCY(scanner parser)

ADD_library(debugger
	awin.c awinch.c debugger.c eval.c cmd.c ui.c
	keywords.c decode.c
	${BISON_parser_OUTPUTS}
	${FLEX_scanner_OUTPUTS}
)

strdup(scanner.c)
strdup(awin.c)
strdup(cmd.c)
strdup(eval.c)
strdup(log.c)

# vim: tabstop=4
