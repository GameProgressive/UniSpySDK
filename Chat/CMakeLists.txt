cmake_minimum_required(VERSION 3.0.0)

add_library(rschat
    chatCallbacks.c
    chatChannel.c
    chatCrypt.c
    chatHandlers.c
    chatMain.c
    chatSocket.c
)

target_link_libraries(rschat rscommon)

if(RS_BUILD_TESTS)
    add_executable(chattest chatc/chatc.c)
    target_link_libraries(chattest rschat)
    target_compile_definitions(chattest PRIVATE test_main=main)
endif()
