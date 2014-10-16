CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4)
PROJECT(em400 C)

set(WITH_DEBUGGER 1)

set(EM400_VERSION 0.1.0)

find_package(BISON)
find_package(FLEX)
find_package(Threads)

function(xopen FILE STANDARD)
math(EXPR VER "${STANDARD} / 100")
message(STATUS "${FILE} needs ${ARGN} from X/Open ${VER}")
set_property(
	SOURCE ${FILE}
	APPEND
	PROPERTY COMPILE_DEFINITIONS
	_XOPEN_SOURCE=${STANDARD}
)
endfunction(xopen)

function(strdup FILE)
xopen(${FILE} 500 "strdup()")
endfunction(strdup)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/build)

ADD_DEFINITIONS(-Wall -std=c99 -O3 -DEM400_VERSION="${EM400_VERSION}")

IF(WITH_DEBUGGER)
	set(CURSES_NEED_NCURSES TRUE)
	find_package(Curses)
	include_directories(SYSTEM ${CURSES_INCLUDE_DIR})
	INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/build/debugger) 

	FIND_PATH(READLINE_INCLUDE_DIR readline/readline.h)
	FIND_LIBRARY(READLINE_LIBRARY NAMES readline) 
	include_directories(${READLINE_INCLUDE_DIR}) 

	ADD_DEFINITIONS(-DWITH_DEBUGGER)
	ADD_SUBDIRECTORY(src/debugger build/debugger)
	set(DEBUGGER_LIBS debugger ${CURSES_LIBRARIES} ${READLINE_LIBRARY})
ELSE(WITH_DEBUGGER)
	set(DEBUGGER_LIBS)
ENDIF(WITH_DEBUGGER)

ADD_SUBDIRECTORY(src/io build/io)
ADD_SUBDIRECTORY(src/mem build/mem)
ADD_SUBDIRECTORY(src/cpu build/cpu)
ADD_SUBDIRECTORY(src build)

install(FILES em400.cfg.template DESTINATION /etc/em400 RENAME em400.cfg OPTIONAL)

# vim: tabstop=4
