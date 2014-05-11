enable_testing()

add_executable(test_int2binf
        ${CMAKE_SOURCE_DIR}/src/utils.c test_int2binf.c
)

add_test(test_int2binf ${CMAKE_CURRENT_BINARY_DIR}/test_int2binf)



