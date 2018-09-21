cmake_minimum_required(VERSION 3.0.0)

add_library(rspeer
    peerAutoMatch.c
    peerCallbacks.c
    peerGlobalCallbacks.c
    peerHost.c
    peerKeys.c
    peerMain.c
    peerMangle.c
    peerOperations.c
    peerPing.c
    peerPlayers.c
    peerQR.c
    peerRooms.c
    peerSB.c
)

target_link_libraries(rspeer rscommon rsqr2 rschat rspinger rsserverbrowsing)

if(RS_BUILD_TESTS)
    add_executable(peertest peerc/peerc.c)
    target_link_libraries(peertest rspeer)
    target_compile_definitions(peertest PRIVATE test_main=main)
endif()
