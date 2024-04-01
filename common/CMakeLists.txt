cmake_minimum_required(VERSION 3.0.0)

add_subdirectory(${RS_PLATFORM})

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
    gsPlatformUtil.c
    gsRC4.c
    gsResultCodes.c
    gsSHA1.c
    gsSSL.c
    gsStringUtil.c
    gsXML.c
    hashtable.c
    md5c.c
    darray.h
    gsAssert.h
    gsAvailable.h
    gsCommon.h
    gsCore.h
    gsCrypt.h
    gsDebug.h
    gsLargeInt.h
    gsMemory.h
    gsPlatform.h
    gsPlatformSocket.h
    gsPlatformThread.h
    gsPlatformUtil.h
    gsRC4.h
    gsResultCodes.h
    gsSHA1.h
    gsSoap.h
    gsSSL.h
    gsStringUtil.h
    gsUdpEngine.h
    gsXML.h
    hashtable.h
    md5.h
    nonport.h
	${CMAKE_BINARY_DIR}/generated/rsCommon.h
)

# Handle linking threading library.
find_package(Threads REQUIRED)
list(APPEND RS_PLATFORM_LIBS Threads::Threads)

if (CMAKE_BUILD_TYPE EQUAL "Debug")
	target_compile_definitions(rscommon PUBLIC "GSI_COMMON_DEBUG=1")
endif()

# Handle linking sockets library.
if(RS_PLATFORM MATCHES "win32")
    if(RS_WINSOCK2)
	list(APPEND RS_PLATFORM_LIBS ws2_32)
    else()
	list(APPEND RS_PLATFORM_LIBS wsock32)
    endif()
	
	list(APPEND RS_PLATFORM_LIBS advapi32)
endif()

target_link_libraries(rscommon PUBLIC ${RS_PLATFORM_LIBS} rsplatform)
