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

# Handle linking threading library.
find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  target_compile_options(rscommon PUBLIC "-pthread")
  set(RS_PLATFORM_LIBS "${CMAKE_THREAD_LIBS_INIT}")
endif()

if (CMAKE_BUILD_TYPE EQUAL "Debug")
	target_compile_definitions(rscommon PUBLIC "GSI_COMMON_DEBUG=1")
endif()

# Handle linking sockets library.
if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    if(RS_WINSOCK2)
        set(RS_PLATFORM_LIBS "${RS_PLATFORM_LIBS}" ws2_32)
    else()
        set(RS_PLATFORM_LIBS "${RS_PLATFORM_LIBS}" wsock32)
    endif()
endif()

target_link_libraries(rscommon "${RS_PLATFORM_LIBS}")
