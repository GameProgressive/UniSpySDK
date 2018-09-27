cmake_minimum_required(VERSION 3.0.0)

add_library(rscdkey
    gcdkeyc.c
    gcdkeys.c
)

target_link_libraries(rscdkey rscommon)

if(RS_BUILD_TESTS)
    add_executable(keyclienttest ClientTest/gcdkeyclienttest.c)
    target_link_libraries(keyclienttest rscdkey)
    target_compile_definitions(keyclienttest PRIVATE test_main=main)
    add_executable(keyservertest ServerTest/gcdkeyservertest.c)
    target_link_libraries(keyservertest rscdkey rsqr2)
    target_compile_definitions(keyservertest PRIVATE test_main=main)
endif()
