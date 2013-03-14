CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(assem C)

set(ASSEM_MAJOR_VERSION 0)
set(ASSEM_MINOR_VERSION 6)
set(ASSEM_PATCH_VERSION 0)
set(ASSEM_VERSION ${ASSEM_MAJOR_VERSION}.${ASSEM_MINOR_VERSION}.${ASSEM_PATCH_VERSION})

find_package(BISON)
find_package(FLEX)

ADD_DEFINITIONS(-Wall -std=c99 -D_XOPEN_SOURCE=600 -O3 -pthread -DASSEM_VERSION="${ASSEM_VERSION}" -g)

BISON_TARGET(parser_modern parser_modern.y ${CMAKE_CURRENT_BINARY_DIR}/parser_modern.c COMPILE_FLAGS "-p yy")
FLEX_TARGET(scanner scanner.l ${CMAKE_CURRENT_BINARY_DIR}/scanner.c COMPILE_FLAGS "-P yy")
ADD_FLEX_BISON_DEPENDENCY(scanner parser_modern)

ADD_EXECUTABLE(assem main.c elements.c eval.c ops.c
${BISON_parser_modern_OUTPUTS}
${FLEX_scanner_OUTPUTS}
)

# vim: tabstop=4
