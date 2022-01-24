add_library(uscdkey
    gcdkeyc.c
    gcdkeys.c
    gcdkeys.h
    gcdkeyc.h
)

target_link_libraries(uscdkey PUBLIC uscommon)

if(UNISPY_BUILD_TESTS)
    add_subdirectory(CdkeyGen)
    add_subdirectory(ClientTest)
    add_subdirectory(ServerTest)
    add_subdirectory(ServerTestQR2)
endif()
