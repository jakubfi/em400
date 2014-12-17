cmake_minimum_required(VERSION 2.8.4)
project(em400 C)

set(WITH_DEBUGGER 1)

set(EM400_VERSION 0.2.1)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(Threads REQUIRED)
find_package(emdas REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_BINARY_DIR}/src)

add_definitions(-Wall -g -std=c99 -O3 -DEM400_VERSION="${EM400_VERSION}")

if(WITH_DEBUGGER)
	set(CURSES_NEED_NCURSES TRUE)
	find_package(Curses REQUIRED)
	include_directories(SYSTEM ${CURSES_INCLUDE_DIR})
	include_directories(${CMAKE_BINARY_DIR}/debugger)

	find_path(READLINE_INCLUDE_DIR readline/readline.h)
	find_library(READLINE_LIBRARY NAMES readline)
	include_directories(${READLINE_INCLUDE_DIR})

	add_definitions(-DWITH_DEBUGGER)
	set(DEBUGGER_LIBS debugger ${CURSES_LIBRARIES} ${READLINE_LIBRARY})
else()
	set(DEBUGGER_LIBS)
endif()

add_subdirectory(src)

# vim: tabstop=4
