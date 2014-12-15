BISON_TARGET(cfg_parser cfg_parser.y ${CMAKE_CURRENT_BINARY_DIR}/cfg_parser.c COMPILE_FLAGS "-p cyy")
FLEX_TARGET(cfg_scanner cfg_scanner.l ${CMAKE_CURRENT_BINARY_DIR}/cfg_scanner.c COMPILE_FLAGS "-P cyy --never-interactive")
ADD_FLEX_BISON_DEPENDENCY(cfg_scanner cfg_parser)

ADD_EXECUTABLE(em400
	em400.c errors.c utils.c cfg.c log.c
	${BISON_cfg_parser_OUTPUTS}
	${FLEX_cfg_scanner_OUTPUTS}
)

ADD_EXECUTABLE(emitool
	emitool.c io/dev/e4image.c
)

TARGET_LINK_LIBRARIES(em400
	rt m
	${CMAKE_THREAD_LIBS_INIT}
	${FLEX_LIBRARIES} ${BISON_LIBRARIES}
	cpu mem io dev
	${EMDAS_LIBRARIES}
	${DEBUGGER_LIBS}
)

ADD_SUBDIRECTORY(io)
ADD_SUBDIRECTORY(mem)
ADD_SUBDIRECTORY(cpu)

IF(WITH_DEBUGGER)
	ADD_SUBDIRECTORY(debugger)
ENDIF(WITH_DEBUGGER)

INSTALL(TARGETS em400 emitool RUNTIME DESTINATION bin)

# vim: tabstop=4
