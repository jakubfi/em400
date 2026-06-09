cmake_minimum_required(VERSION 3.10)
project(em400
	DESCRIPTION "MERA-400 emulator"
	LANGUAGES C CXX
)

include(GNUInstallDirs)
include(cmake/appversion.cmake)

# ---- UI curses ---------------------------------------------------------

find_package(Curses)
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUV REQUIRED IMPORTED_TARGET libuv)

set(UI_CURSES OFF)
if(CURSES_FOUND)
	if(WIN32)
		message(STATUS "Curses library found, but curses UI is disabled on Windows")
	else()
		set(UI_CURSES ON)
		message(STATUS "Curses library found. Curses UI enabled.")
	endif()
else()
	message(STATUS "Curses library not found. Curses UI disabled.")
endif()

if(UI_CURSES)
	set(CURSES_NEED_NCURSES TRUE)
	find_path(READLINE_INCLUDE_DIR readline/readline.h)
	find_library(READLINE_LIBRARY readline)
	find_library(TINFO_LIBRARY tinfo)

	bison_target(ui_curses_parser
		src/ui/curses/parser.y
		${CMAKE_CURRENT_BINARY_DIR}/ui_curses_parser.c
	)
	flex_target(ui_curses_scanner
		src/ui/curses/scanner.l
		${CMAKE_CURRENT_BINARY_DIR}/ui_curses_scanner.c
		COMPILE_FLAGS "--never-interactive"
	)
	add_flex_bison_dependency(ui_curses_scanner ui_curses_parser)

	add_library(ui-curses STATIC
		src/ui/curses/awin.c
		src/ui/curses/awin.h
		src/ui/curses/awinch.c
		src/ui/curses/debugger.c
		src/ui/curses/debugger.h
		src/ui/curses/eval.c
		src/ui/curses/eval.h
		src/ui/curses/cmd.c
		src/ui/curses/cmd.h
		src/ui/curses/ui.c
		src/ui/curses/ui.h
		src/ui/curses/keywords.c
		src/ui/curses/keywords.h
		src/ui/curses/decode.c
		src/ui/curses/decode.h
		src/ui/curses/parser.y
		src/ui/curses/scanner.l
		${BISON_ui_curses_parser_OUTPUTS}
		${FLEX_ui_curses_scanner_OUTPUTS}
	)

	target_link_libraries(ui-curses
		${CURSES_LIBRARIES}
		${TINFO_LIBRARY}
		${READLINE_LIBRARY}
		${FLEX_LIBRARIES}
		${BISON_LIBRARIES}
	)

	target_include_directories(ui-curses PRIVATE ${CMAKE_SOURCE_DIR}/src)
	target_include_directories(ui-curses PRIVATE ${CMAKE_SOURCE_DIR}/include)
	target_include_directories(ui-curses PRIVATE ${CMAKE_BINARY_DIR})
endif(UI_CURSES)

# ---- Target: em400 -----------------------------------------------------

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(Threads REQUIRED)
find_package(emdas REQUIRED)
find_package(emcrk REQUIRED)
find_package(emawp 3.0 REQUIRED)
if(NOT WIN32)
	find_library(MATH_LIBRARY m)
	find_library(RT_LIBRARY rt)
endif()

# --- IPO support

include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)
if(IPO_SUPPORTED)
	message(STATUS "LTO supported and enabled")
else()
	message(STATUS "LTO disabled: ${IPO_ERROR}")
endif()

# --- Sound (miniaudio, vendored)

if(NOT WIN32)
	find_library(DL_LIBRARY dl)
endif()

# --- QT

set(UI_QT OFF)
find_package(Qt6 COMPONENTS Widgets)
find_package(Qt6 COMPONENTS Multimedia)

if (Qt6Widgets_FOUND)
	message(STATUS "Found QT: ${Qt6Widgets_VERSION_STRING}")
	set(CMAKE_CXX_STANDARD 11)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(UI_QT ON)
else()
	message(STATUS "QT not found")
endif()

# --- EM400

set(SOURCES_EM400
	src/em400.c
	src/log.c
	src/log.h
	src/log_io.h
	src/log_crk.c
	src/log_crk.h
	src/cfg.c
	src/cfg.h

	src/utils/utils.c
	src/utils/utils.h
	src/utils/elst.c
	src/utils/elst.h
	src/utils/serial.c
	src/utils/serial.h

	src/cpu/instructions.c
	src/cpu/instructions.h
	src/cpu/clock.c
	src/cpu/clock.h
	src/cpu/cpu.c
	src/cpu/cpu.h
	src/cpu/interrupts.c
	src/cpu/interrupts.h
	src/cpu/iset.c
	src/cpu/iset.h
	src/cpu/alu.c
	src/cpu/alu.h

	src/mem/mem.c
	src/mem/mem.h
	src/mem/mega.c
	src/mem/mega.h
	src/mem/mega_prom.c
	src/mem/mega_prom.h
	src/mem/elwro.c
	src/mem/elwro.h

	#	src/fpga/iobus.c
	#	src/fpga/iobus.h

	src/io/defs.h
	src/io/io.c
	src/io/io.h
	src/io/chan.c
	src/io/chan.h
	src/io/cchar/cchar.c
	src/io/cchar/cchar.h
	src/io/cchar/uzdat.c
	src/io/cchar/uzdat.h
	src/io/cchar/uzfx.c
	src/io/cchar/uzfx.h
	src/io/cchar/meclo.c
	src/io/cchar/meclo.h
	src/io/cchar/meclo_prom.c
	src/io/cchar/meclo_prom.h
	src/io/iotester.c
	src/io/iotester.h

	src/io/dev/dev.h
	src/io/dev/e4image.c
	src/io/dev/e4image.h
	src/io/dev/terminal.c
	src/io/dev/terminal.h
	src/io/dev/sp45de.c
	src/io/dev/sp45de.h
	src/io/dev/winchester.c
	src/io/dev/winchester.h
	src/io/dev/flop5.c
	src/io/dev/flop5.h
	src/io/dev/rtclock.h
	src/io/dev/rtclock.c

	src/io/mx/mx.c
	src/io/mx/mx.h
	src/io/mx/cmds.c
	src/io/mx/cmds.h
	src/io/mx/irq.c
	src/io/mx/irq.h
	src/io/mx/event.c
	src/io/mx/event.h
	src/io/mx/line.c
	src/io/mx/line.h
	src/io/mx/proto_common.c
	src/io/mx/proto_common.h
	src/io/mx/proto_winchester.c
	src/io/mx/proto_floppy.c
	src/io/mx/proto_tape.c
	src/io/mx/proto_puncher.c
	src/io/mx/proto_punchrd.c
	src/io/mx/proto_terminal.c
	src/io/mx/proto_som_puncher.c
	src/io/mx/proto_som_punchrd.c
	src/io/mx/proto_som_terminal.c

	src/ui/ui.c
	src/ui/ui.h

	src/ui/cmd/cmd.c
	src/ui/cmd/commands.c
	src/ui/cmd/commands.h
	src/ui/cmd/utils.c
	src/ui/cmd/utils.h

	src/cp/cp.c
	src/cp/cp.h
	src/cp/eval.c
	src/cp/eval.h
	src/cp/eval_parser.y
	src/cp/eval_scanner.l
	src/cp/brk.c
	src/cp/brk.h
	src/cp/watch.c
	src/cp/watch.h

	src/sound/sound.c
	src/sound/sound.h
	src/external/miniaudio/miniaudio.c
	src/external/miniaudio/miniaudio.h
	src/cpu/buzzer.c
	src/cpu/buzzer.h

	src/external/iniparser/dictionary.c
	src/external/iniparser/dictionary.h
	src/external/iniparser/iniparser.c
	src/external/iniparser/iniparser.h

	src/libem400.c
	src/libem400.h
)

if (UI_QT)
	list(APPEND SOURCES_EM400 src/ui/qt6/qt6.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt6/resources.qrc)

	# application shell + shared infrastructure
	list(APPEND SOURCES_EM400 src/ui/qt6/app/mainwindow.ui)
	list(APPEND SOURCES_EM400 src/ui/qt6/app/mainwindow.h src/ui/qt6/app/mainwindow.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt6/app/emumodel.h src/ui/qt6/app/emumodel.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt6/app/theme.h src/ui/qt6/app/theme.cpp)

	# dockable debugger views
	list(APPEND SOURCES_EM400 src/ui/qt6/views/dasmview.h src/ui/qt6/views/dasmview.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt6/views/asmline.h src/ui/qt6/views/asmline.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt6/views/memview.h src/ui/qt6/views/memview.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt6/views/memsearch.h src/ui/qt6/views/memsearch.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt6/views/regview.h src/ui/qt6/views/regview.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt6/views/intview.h src/ui/qt6/views/intview.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt6/views/mapview.h src/ui/qt6/views/mapview.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt6/views/brkview.h src/ui/qt6/views/brkview.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt6/views/watchview.h src/ui/qt6/views/watchview.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt6/views/stackview.h src/ui/qt6/views/stackview.cpp)

	# physical control panel (pulpit) replica
	list(APPEND SOURCES_EM400 src/ui/qt6/panel/controlpanel.h src/ui/qt6/panel/controlpanel.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt6/panel/switch.h src/ui/qt6/panel/switch.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt6/panel/led.h src/ui/qt6/panel/led.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt6/panel/buswleds.h src/ui/qt6/panel/buswleds.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt6/panel/binarykeys.h src/ui/qt6/panel/binarykeys.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt6/panel/rotary.h src/ui/qt6/panel/rotary.cpp)
	list(APPEND SOURCES_EM400 src/ui/qt6/panel/ignition.h src/ui/qt6/panel/ignition.cpp)
endif()

# --- internal parser

bison_target(eval_parser
	src/cp/eval_parser.y
	${CMAKE_CURRENT_BINARY_DIR}/eval_parser.c
	DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/eval_parser.h
	COMPILE_FLAGS "-p eval_yy"
)
flex_target(eval_scanner
	src/cp/eval_scanner.l
	${CMAKE_CURRENT_BINARY_DIR}/eval_scanner.c
	COMPILE_FLAGS "-P eval_yy --never-interactive"
)
add_flex_bison_dependency(eval_scanner eval_parser)

# "%define parse.error custom" makes bison emit a yypcontext_expected_tokens()
# helper for building "expecting ..." lists; our custom handler reports by token
# kind only and leaves it unused. Silence the resulting -Wunused-function for
# this generated file alone (we do not control its contents).
set_source_files_properties(
	${CMAKE_CURRENT_BINARY_DIR}/eval_parser.c
	PROPERTIES COMPILE_OPTIONS "-Wno-unused-function"
)

# --- em400 target

add_executable(em400
	${SOURCES_EM400}
	${BISON_eval_parser_OUTPUTS}
	${FLEX_eval_scanner_OUTPUTS}
)

set_property(TARGET em400 PROPERTY C_STANDARD 23)
target_compile_options(em400 PRIVATE -Wall)
target_include_directories(em400 PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(em400 PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(em400 PRIVATE ${CMAKE_BINARY_DIR})
target_include_directories(em400 PRIVATE ${LIBUV_INCLUDE_DIRS})
target_compile_definitions(em400 PRIVATE EM400_VERSION="${APP_VERSION}")
if(UI_CURSES)
	target_compile_definitions(em400 PRIVATE UI_CURSES)
	target_link_libraries(em400 ui-curses)
endif(UI_CURSES)

if(IPO_SUPPORTED)
	set_property(TARGET em400 PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

if (UI_QT)
	add_compile_definitions($<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>)
	set_property(TARGET em400 PROPERTY AUTOMOC ON)
	set_property(TARGET em400 PROPERTY AUTOUIC ON)
	set_property(TARGET em400 PROPERTY AUTORCC ON)
	set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/eval_parser.c PROPERTY SKIP_AUTOGEN ON)
	set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/eval_parser.h PROPERTY SKIP_AUTOGEN ON)
	set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/eval_scanner.c PROPERTY SKIP_AUTOGEN ON)
	set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/uicurses_parser.c PROPERTY SKIP_AUTOGEN ON)
	set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/uicurses_parser.h PROPERTY SKIP_AUTOGEN ON)
	set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/uicurses_scanner.c PROPERTY SKIP_AUTOGEN ON)
	target_include_directories(em400 PRIVATE ${Qt6Widgets_INCLUDE_DIRS})
	# qt6 sources live in subdirs (app/, panel/, views/) but include each
	# other with bare "foo.h" includes; expose the subdirs on the search path
	target_include_directories(em400 PRIVATE
		${CMAKE_SOURCE_DIR}/src/ui/qt6
		${CMAKE_SOURCE_DIR}/src/ui/qt6/app
		${CMAKE_SOURCE_DIR}/src/ui/qt6/panel
		${CMAKE_SOURCE_DIR}/src/ui/qt6/views)
	target_compile_definitions(em400 PRIVATE UI_QT)
	target_link_libraries(em400 ${Qt6Widgets_LIBRARIES})
	target_link_libraries(em400 ${Qt6Multimedia_LIBRARIES})
endif()

target_link_libraries(em400
	emdas
	emcrk
	emawp
	${FLEX_LIBRARIES}
	${BISON_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	PkgConfig::LIBUV
)

if(NOT WIN32)
	target_link_libraries(em400
		${RT_LIBRARY}
		${MATH_LIBRARY}
	)
endif()

# miniaudio: silence -Wall noise in the vendored implementation TU, and link
# libdl on Linux so its runtime loader for ALSA/PulseAudio/JACK works.
set_source_files_properties(src/external/miniaudio/miniaudio.c
	PROPERTIES COMPILE_FLAGS "-w")
if(NOT WIN32 AND DL_LIBRARY)
	target_link_libraries(em400 ${DL_LIBRARY})
endif()

install(TARGETS em400
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
	${CMAKE_SOURCE_DIR}/tools/e4term
	PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE
				OWNER_READ GROUP_READ WORLD_READ
				OWNER_WRITE
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ---- Target: emitool ---------------------------------------------------

add_executable(emitool
	src/emitool.c
	src/io/dev/e4image.c
)
if(WIN32)
	target_link_libraries(emitool ws2_32)
endif()
set_property(TARGET emitool PROPERTY C_STANDARD 23)
target_include_directories(emitool PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_options(emitool PUBLIC -Wall)

install(TARGETS emitool
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ---- Target: embin -----------------------------------------------------

add_executable(embin
	src/embin.c
	src/utils/utils.c
)
if(NOT WIN32)
	target_sources(embin PRIVATE src/utils/serial.c)
else()
	target_link_libraries(embin ws2_32)
endif()
set_property(TARGET embin PROPERTY C_STANDARD 23)
target_include_directories(embin PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_options(embin PUBLIC -Wall)

install(TARGETS embin
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# vim: tabstop=4
