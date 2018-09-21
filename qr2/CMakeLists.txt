cmake_minimum_required(VERSION 3.0.0)

add_library(rsqr2
    qr2.c
    qr2regkeys.c
)

target_link_libraries(rsqr2 rscommon rsnatneg)

if(RS_BUILD_TESTS)
    add_executable(qr2sample qr2csample/qr2csample.c)
    target_link_libraries(qr2sample rsqr2)
    target_compile_definitions(qr2sample PRIVATE test_main=main)
endif()
