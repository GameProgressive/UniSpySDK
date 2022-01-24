add_library(usgt2
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
    gt2.h
    gt2Auth.h
    gt2Buffer.h
    gt2Callback.h
    gt2Connection.h
    gt2Encode.h
    gt2Filter.h
    gt2Main.h
    gt2Message.h
    gt2Socket.h
    gt2Utility.h
)

target_link_libraries(usgt2 PUBLIC uscommon)

if(UNISPY_BUILD_TESTS)
    add_subdirectory(gt2hostmig)
    add_subdirectory(gt2testc)
    add_subdirectory(gt2nat)
    add_subdirectory(gt2proxy)

    if (UNISPY_WINDOWS)
        add_subdirectory(gt2test)
    endif()
endif()
