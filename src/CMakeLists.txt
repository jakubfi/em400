BISON_TARGET(cfg_parser cfg_parser.y ${CMAKE_CURRENT_BINARY_DIR}/cfg_parser.c COMPILE_FLAGS "-p cyy")
FLEX_TARGET(cfg_scanner cfg_scanner.l ${CMAKE_CURRENT_BINARY_DIR}/cfg_scanner.c COMPILE_FLAGS "-P cyy")
ADD_FLEX_BISON_DEPENDENCY(cfg_scanner cfg_parser)

# POSIX: em400.c needs IEEE Std 1003.2-1992 getopt(3)
# POSIX: cfg_parser.c needs X/Open 5 fileno(2)
# POSIX: cfg_scanner.c needs X/Open 5 fileno(2)
set_property(
	SOURCE em400.c
	SOURCE cfg_parser.c
	SOURCE cfg_scanner.c
	APPEND
	PROPERTY COMPILE_DEFINITIONS
	_XOPEN_SOURCE=500
)

ADD_EXECUTABLE(em400
	em400.c errors.c utils.c cfg.c
	${BISON_cfg_parser_OUTPUTS}
	${FLEX_cfg_scanner_OUTPUTS}
)

ADD_EXECUTABLE(emitool
	emitool.c io/e4image.c
)

target_link_libraries(em400
	rt m
	${CMAKE_THREAD_LIBS_INIT}
	${FLEX_LIBRARIES} ${BISON_LIBRARIES}
	${DEBUGGER_LIBS}
	cpu mem io
)

install(TARGETS em400 emitool RUNTIME DESTINATION bin)

# vim: tabstop=4
