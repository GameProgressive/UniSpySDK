add_library(usgp
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
    gp.h
    gpi.h
    gpiBuddy.h
    gpiBuffer.h
    gpiCallback.h
    gpiConnect.h
    gpiInfo.h
    gpiKeys.h
    gpiOperation.h
    gpiPeer.h
    gpiProfile.h
    # gpiPS3.h
    gpiSearch.h
    gpiTransfer.h
    gpiUnique.h
    gpiUtility.h
)

target_link_libraries(usgp PUBLIC usgt2)

if(UNISPY_BUILD_TESTS)
    #add_subdirectory(gpstress) ##TODO
    add_subdirectory(gptestc)

    if (UNISPY_WINDOWS)
        add_subdirectory(gptest)
    endif()
endif()
