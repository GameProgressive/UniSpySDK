cmake_minimum_required(VERSION 3.0.0)

add_library(rsgp
    gp.c
    gpi.c
    gpiBuddy.c
    gpiBuffer.c
    gpiCallback.c
    gpiConnect.c
    gpiInfo.c
    gpiKeys.c
    gpiOperation.c
    gpiPeer.c
    gpiProfile.c
    # Should be build only for PS3.
    # gpiPS3.c
    gpiSearch.c
    gpiTransfer.c
    gpiUnique.c
    gpiUtility.c
)

target_link_libraries(rsgp rsgt2)

if(RS_BUILD_TESTS)
    add_executable(gptest gptestc/gptestc.c)
    target_link_libraries(gptest rsgp)
    target_compile_definitions(gptest PRIVATE test_main=main)
endif()
