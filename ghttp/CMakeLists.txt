cmake_minimum_required(VERSION 3.0.0)

add_library(rshttp
    ghttpBuffer.c
    ghttpc/ghttpc.c
    ghttpCallbacks.c
    ghttpCommon.c
    ghttpConnection.c
    ghttpEncryption.c
    ghttpMain.c
    ghttpPost.c
    ghttpProcess.c
)

