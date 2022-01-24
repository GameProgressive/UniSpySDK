add_library(usserverbrowsing
    sb_crypt.c
    sb_queryengine.c
    sb_server.c
    sb_serverbrowsing.c
    sb_serverlist.c
    sb_internal.h
    sb_serverbrowsing.h
    sb_crypt.h
    sb_ascii.h
)

target_link_libraries(usserverbrowsing PUBLIC usqr2)

if(UNISPY_BUILD_TESTS)
    add_subdirectory(sbctest)
    add_subdirectory(querytest)
    
    if (UNISPY_WINDOWS)
        add_subdirectory(sbmfcsample)
    endif()
endif()
