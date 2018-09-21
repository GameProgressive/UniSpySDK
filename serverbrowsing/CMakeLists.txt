cmake_minimum_required(VERSION 3.0.0)

add_library(rsserverbrowsing
    sb_crypt.c
    sb_queryengine.c
    sb_server.c
    sb_serverbrowsing.c
    sb_serverlist.c
)

target_link_libraries(rsserverbrowsing rscommon rsqr2)

if(RS_BUILD_TESTS)
    add_executable(sbtest sbctest/sbctest.c)
    target_link_libraries(sbtest rsserverbrowsing)
    target_compile_definitions(sbtest PRIVATE test_main=main)
endif()
