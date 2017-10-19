bison_target(cfg_parser cfg_parser.y ${CMAKE_CURRENT_BINARY_DIR}/cfg_parser.c COMPILE_FLAGS "-p cyy")
flex_target(cfg_scanner cfg_scanner.l ${CMAKE_CURRENT_BINARY_DIR}/cfg_scanner.c COMPILE_FLAGS "-P cyy --never-interactive")
add_flex_bison_dependency(cfg_scanner cfg_parser)

add_executable(em400
	em400.c utils.c cfg.c log.c log_crk.c log_io.c
	${BISON_cfg_parser_OUTPUTS}
	${FLEX_cfg_scanner_OUTPUTS}
)

add_executable(emitool
	emitool.c io/dev/e4image.c
)

include_directories(
	${EMDAS_INCLUDE_DIRS}
	${EMCRK_INCLUDE_DIRS}
	${EMAWP_INCLUDE_DIRS}
	${FLEX_INCLUDE_DIRS}
	${BISON_INCLUDE_DIRS}
)
target_link_libraries(em400
	ectl cpu mem io ui
	${DEBUGGER_LIBS}
	${EMDAS_LIBRARIES}
	${EMCRK_LIBRARIES}
	${EMAWP_LIBRARIES}
	${FLEX_LIBRARIES} ${BISON_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	rt m
)

add_subdirectory(io)
add_subdirectory(mem)
add_subdirectory(cpu)
add_subdirectory(ectl)
add_subdirectory(ui)

if(WITH_DEBUGGER)
	add_subdirectory(debugger)
endif(WITH_DEBUGGER)

install(TARGETS em400 emitool
	RUNTIME DESTINATION bin
)

# vim: tabstop=4
