cmake_minimum_required(VERSION 3.0.0)

add_library(rssake
    sakeMain.c
    sakeRequest.c
    sakeRequestMisc.c
    sakeRequestModify.c
    sakeRequestRead.c
    ../common/gsSoap.c
)

target_link_libraries(rssake rscommon rshttp)

if(RS_BUILD_TESTS)
    add_executable(saketest saketest/saketest.c)
    target_link_libraries(saketest rssake rsgp rsgt2)
    target_compile_definitions(saketest PRIVATE test_main=main)
endif()
