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
    gsSSL.c
    gsStringUtil.c
    gsXML.c
    hashtable.c
    md5c.c
)

find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  target_compile_options(rscommon PUBLIC "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(rscommon "${CMAKE_THREAD_LIBS_INIT}")
endif()
