cmake_minimum_required(VERSION 3.0.0)

add_library(rsgt2
    gt2Auth.c
    gt2Buffer.c
    gt2Callback.c
    gt2Connection.c
    gt2Encode.c
    gt2Filter.c
    gt2Main.c
    gt2Message.c
    gt2Socket.c
    gt2Utility.c
    ../common/gsUdpEngine.c
)

target_link_libraries(rsgt2 rscommon)

if(RS_BUILD_TESTS)
    add_executable(gt2test gt2testc/gt2testc.c)
    target_link_libraries(gt2test rsgt2)
    target_compile_definitions(gt2test PRIVATE test_main=main)
endif()
