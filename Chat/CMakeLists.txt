add_library(uschat
    chatCallbacks.c
    chatChannel.c
    chatCrypt.c
    chatHandlers.c
    chatMain.c
    chatSocket.c
    chat.h
    chatASCII.h
    chatCallbacks.h
    chatChannel.h
    chatCrypt.h
    chatHandlers.h
    chatMain.h
    chatSocket.h
)

target_link_libraries(uschat PUBLIC uscommon)

if(UNISPY_BUILD_TESTS)
    add_subdirectory(chatc)

    if (UNISPY_WINDOWS)
        add_subdirectory(chatty)
    endif()
endif()
