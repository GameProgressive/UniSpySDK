add_library(uspeer
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
    peer.h
    peerAscii.h
    peerAutoMatch.h
    peerCallbacks.h
    peerGlobalCallbacks.h
    peerHost.h
    peerKeys.h
    peerMain.h
    peerMangle.h
    peerOperations.h
    peerPing.h
    peerPlayers.h
    peerQR.h
    peerRooms.h
    peerSB.h
)

target_link_libraries(uspeer PUBLIC usqr2 uschat uspinger usserverbrowsing)

if(UNISPY_BUILD_TESTS)
    add_subdirectory(peerc)

    if (UNISPY_WINDOWS)
        add_subdirectory(PeerLobby)
        add_subdirectory(PeerTest)
    endif()
endif()
