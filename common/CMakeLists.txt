set(COMMON_SRC
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
    gsSSL.h
    gsStringUtil.h
    gsUdpEngine.h
    gsUdpEngine.c
    gsXML.h
    hashtable.h
    md5.h
    nonport.h
)

if (UNISPY_WINDOWS)
    list(APPEND COMMON_SRC win32/Win32Common.c)
endif()
if (UNISPY_MACOSX)
    list(APPEND COMMON_SRC macosx/MacOSXCommon.c)
endif()
if (UNISPY_LINUX)
    list(APPEND COMMON_SRC linux/LinuxCommon.c)
endif()

add_library(uscommon ${COMMON_SRC})

# Handle linking threading library.
find_package(Threads REQUIRED)
list(APPEND UNISPY_PLATFORM_LIBS Threads::Threads)

if (CMAKE_BUILD_TYPE EQUAL "Debug")
	target_compile_definitions(uscommon PUBLIC "GSI_COMMON_DEBUG=1")
endif()

# Handle linking sockets library.
if (UNISPY_WINDOWS)
	list(APPEND UNISPY_PLATFORM_LIBS ${UNISPY_WINSOCK_LIB})
endif()

target_link_libraries(uscommon PUBLIC ${UNISPY_PLATFORM_LIBS} usgt2)
