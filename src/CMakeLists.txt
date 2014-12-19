bison_target(cfg_parser cfg_parser.y ${CMAKE_CURRENT_BINARY_DIR}/cfg_parser.c COMPILE_FLAGS "-p cyy")
flex_target(cfg_scanner cfg_scanner.l ${CMAKE_CURRENT_BINARY_DIR}/cfg_scanner.c COMPILE_FLAGS "-P cyy --never-interactive")
add_flex_bison_dependency(cfg_scanner cfg_parser)

add_executable(em400
	em400.c errors.c utils.c cfg.c log.c
	${BISON_cfg_parser_OUTPUTS}
	${FLEX_cfg_scanner_OUTPUTS}
)

add_executable(emitool
	emitool.c io/dev/e4image.c
)

target_link_libraries(em400
	rt m
	${CMAKE_THREAD_LIBS_INIT}
	${FLEX_LIBRARIES} ${BISON_LIBRARIES}
	cpu mem io dev
	${EMDAS_LIBRARIES}
	${EMCRK_LIBRARIES}
	${DEBUGGER_LIBS}
)

add_subdirectory(io)
add_subdirectory(mem)
add_subdirectory(cpu)

if(WITH_DEBUGGER)
	add_subdirectory(debugger)
endif(WITH_DEBUGGER)

install(TARGETS em400 emitool
	RUNTIME DESTINATION bin
)

# vim: tabstop=4
