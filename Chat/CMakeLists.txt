cmake_minimum_required(VERSION 3.0.0)

add_library(rschat
    chatCallbacks.c
    chatChannel.c
    chatCrypt.c
    chatHandlers.c
    chatMain.c
    chatSocket.c
)
