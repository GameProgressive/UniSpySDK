cmake_minimum_required(VERSION 3.0.0)

add_library(rspt
    ptMain.c
)

target_link_libraries(rspt rscommon rshttp)

if(RS_BUILD_TESTS)
    add_executable(pttest pttestc/pttestc.c)
    target_link_libraries(pttest rspt)
    target_compile_definitions(pttest PRIVATE test_main=main)
endif()
