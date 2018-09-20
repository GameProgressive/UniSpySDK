cmake_minimum_required(VERSION 3.0.0)

add_library(rscommon
    darray.c
    gsAssert.c
    gsAvailable.c
    gsCore.c
    gsCrypt.c
    gsDebug.c
    gsLargeInt.c
    gsMemory.c
    gsPlatform.c
    gsPlatformSocket.c
    gsPlatformThread.c
    gsPlatformUtil.c
    gsRC4.c
    gsResultCodes.c
    gsSHA1.c
    gsSoap.c
    gsSSL.c
    gsStringUtil.c
    gsUdpEngine.c
    gsXML.c
    hashtable.c
    md5c.c
)
