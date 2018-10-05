cmake_minimum_required(VERSION 3.0.0)

add_library(rshttp
    ghttpBuffer.c
    ghttpc/ghttpc.c
    ghttpCallbacks.c
    ghttpCommon.c
    ghttpConnection.c
    ghttpEncryption.c
    ghttpMain.c
    ghttpPost.c
    ghttpProcess.c
    ../common/gsSoap.c
)

target_link_libraries(rshttp rscommon)

if(RS_BUILD_TESTS)
    add_executable(httptest ghttpc/ghttpc.c)
    target_link_libraries(httptest rshttp)
    target_compile_definitions(httptest PRIVATE test_main=main)
endif()
