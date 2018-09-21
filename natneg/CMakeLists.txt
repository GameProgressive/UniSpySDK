cmake_minimum_required(VERSION 3.0.0)

add_library(rsnatneg
    NATify.c
    natneg.c
)

target_link_libraries(rsnatneg rscommon)

if(RS_BUILD_TESTS)
    add_executable(natnegtest simpletest/simpletest.c)
    target_link_libraries(natnegtest rsnatneg)
    target_compile_definitions(natnegtest PRIVATE test_main=main)
endif()
