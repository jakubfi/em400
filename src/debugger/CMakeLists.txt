bison_target(parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.c)
flex_target(scanner scanner.l ${CMAKE_CURRENT_BINARY_DIR}/scanner.c COMPILE_FLAGS "--never-interactive")
add_flex_bison_dependency(scanner parser)

add_library(debugger
	awin.c awinch.c debugger.c eval.c cmd.c ui.c
	keywords.c decode.c
	${BISON_parser_OUTPUTS}
	${FLEX_scanner_OUTPUTS}
)

# vim: tabstop=4
